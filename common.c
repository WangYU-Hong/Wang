#include <stdio.h>
#include "unp.h"
#include "common.h"

ssize_t serialize_climsg(const struct climsg *msg, void *buf, size_t buflen)

{
    int buflenerr = 0, climsgerr = 0;
    size_t pktlen;
    switch (msg->type)
    {
    case ANSWER:
        pktlen = 11;
        if (buflen < pktlen)
        {
            buflenerr = 1;
            goto fail;
        }
        memset(buf, 0, pktlen);
        int nbytes = sizeof(char);
        void *cur = buf;
        memcpy(cur, (char *)&msg->type, nbytes);
        cur += nbytes;
        nbytes = sizeof(msg->ans);
        memcpy(cur, &msg->ans, nbytes);
        cur += nbytes;
        nbytes = sizeof(msg->anstime);
        memcpy(cur, &msg->anstime, nbytes);
        cur += nbytes;
        nbytes = sizeof(PKTEND);
        memcpy(cur, PKTEND, nbytes);
        cur += nbytes;
        break;

    case MENU:
        pktlen = 3;
        if (buflen < pktlen)
        {
            buflenerr = 1;
            goto fail;
        }

        snprintf(buf, buflen, "%c%c%s",
                 msg->type,
                 msg->menuopt,
                 PKTEND);
        break;

    default:
        climsgerr = 1;
        goto fail;
        break;
    }
    return pktlen;

fail:
    printf("%s error, ", __func__);
    if (buflenerr)
    {
        puts("buflen too short");
    }
    else if (climsgerr)
    {
        puts("invalid climsg");
    }
    return -1;
}

int deserialize_climsg(struct climsg *msg, const void *buf, size_t pktlen)
{
    char *bytebuf = (char *)buf;
    if (pktlen <= 0)
        goto fail;

    memset(msg, 0, sizeof(struct climsg));

    char type = bytebuf[0];
    switch (type)
    {
    case ANSWER: // ans, anstime
        if (pktlen != 11)
            goto fail;
        char ans = bytebuf[1];
        time_t *anstime = (time_t *)&bytebuf[2];

        msg->ans = ans;
        msg->anstime = *anstime;
        break;

    case MENU:
        if (pktlen != 3)
            goto fail;
        char menuopt = bytebuf[1];
        msg->menuopt = menuopt;
        break;

    default:
        goto fail;
    }
    msg->type = type;
    return 0;

fail:
    printf("%s error: invalid pkt\n", __func__);
    return -1;
}

void print_climsg(const struct climsg *msg)
{
    printf("type=%c, ans=%c, anstime=%ld, menuopt=%c\n",
           msg->type,
           msg->ans,
           msg->anstime,
           msg->menuopt);
}

ssize_t mvcurwcpy(wchar_t **cur, const wchar_t *src, size_t *buflen)
{
    size_t nwchar = wcslen(src);
    buflen -= nwchar * sizeof(wchar_t);
    if (buflen < 0)
        return -1;
    wmemcpy(*cur, src, nwchar);
    *cur += nwchar;
    return nwchar * sizeof(wchar_t);
}

ssize_t serialize_question(const struct question *questions, size_t numq, void *buf, size_t buflen)
{
    wchar_t *cur = buf;
    ssize_t pktlen = 0;
    ssize_t n;
    for (int i = 0; i < numq; i++)
    {
        const struct question *currq = &questions[i];

        // q
        n = mvcurwcpy(&cur, currq->q, &buflen);
        if (n < 0)
            goto fail;
        pktlen += n;
        // ","
        n = mvcurwcpy(&cur, DELIM, &buflen);
        if (n < 0)
            goto fail;
        pktlen += n;
        // options
        for (int j = 0; j < OPTIONNUM; j++)
        {
            n = mvcurwcpy(&cur, currq->option[j], &buflen);
            if (n < 0)
                goto fail;
            pktlen += n;
            if ((i + 1) == numq && (j + 1) == OPTIONNUM)
                break;
            n = mvcurwcpy(&cur, DELIM, &buflen);
            if (n < 0)
                goto fail;
            pktlen += n;
        }
    }
    return pktlen;

fail:
    printf("%s error, not enough buffer\n", __func__);
    return -1;
}

int deserialize_question(struct question *questions, size_t numq, const void *buf, size_t pktlen)
{
    const wchar_t* cur = buf;
    int nwchar = 0, cpyoptions = 0, i = 0, readq = 0;
    while (readq < numq && (nwchar * sizeof(wchar_t)) < pktlen) {
        if (*(char*)cur != PKTEND || cur[nwchar++] != DELIM) continue;

        // q
        if (!cpyoptions) {
            wmemcpy(questions->q, cur, nwchar);
            questions->q[nwchar] = L'0'; // null terminate
            cpyoptions = 1;
        }
        else { // cpy options
            wmemcpy(questions->option[i], cur, nwchar);
            questions->option[i][nwchar] = L'0';
            if (++i >= OPTIONNUM) { // question complete
                i = 0;
                cpyoptions = 0;
                readq++;
            }
        }
        cur += nwchar + 1; // skip delim
        nwchar = 0;
    }
    if (readq != numq) {
        printf("%s error: invalid pkt\n", __func__);
        return -1;
    }
    return 0;
}

ssize_t serialize_servmsg(const struct servmsg *msg, void *buf, size_t buflen)
{
    int pktlen = 0;
    ssize_t n;
    char type;
    char *cur;
    switch (msg->type)
    {
    case INIT_2P: // numq, qs
        if (buflen < 2)
            goto fail;
        type = msg->type;
        memcpy(buf, &type, sizeof(char));
        pktlen += sizeof(char);
        n = serialize_question(msg->questions, msg->numq, buf + 1, buflen - pktlen);
        if (n < 0)
            goto fail;
        pktlen += n;
        strcat(buf + pktlen, PKTEND);
        pktlen += 1;
        break;

    case EVAL_ANS:
        if (buflen < 4 + sizeof(int))
            goto fail;

        type = msg->type;
        cur = buf;
        n = sizeof(type);
        memcpy(cur, &type, n);
        cur += n;

        n = sizeof(msg->player);
        memcpy(cur, &msg->player, n);
        cur += n;

        n = sizeof(msg->scorechange);
        memcpy(cur, &msg->scorechange, n);
        cur += n;

        n = sizeof(msg->correct);
        memcpy(cur, &msg->correct, n);
        cur += n;

        n = 1;
        memcpy(cur, PKTEND, n);
        cur += n;
        pktlen = cur - (char *)buf;
        break;

    case GAME_RESULT:
        if (buflen < (2 + msg->numplayer * sizeof(struct player_result)))
            goto fail;

        cur = buf;
        type = msg->type;
        n = sizeof(type);
        memcpy(cur, &type, n);
        cur += n;
        // data section
        for (int i = 0; i < msg->numplayer; i++)
        {
            struct player_result *data = &msg->resultdata[i];
            n = sizeof(data->score);
            memcpy(cur, &data->score, n);
            cur += n;
            n = sizeof(data->coin);
            memcpy(cur, &data->coin, n);
            cur += n;
        }

        n = 1;
        memcpy(cur, PKTEND, n);
        cur += n;
        pktlen = cur - (char *)buf;
        break;

    default:
        break;
    }
    return pktlen;
fail:
    printf("%s error, not enough buffer\n", __func__);
    return -1;
}

int deserialize_servmsg(struct servmsg *msg, const void *buf, size_t pktlen) //TODO
{
    return 0;
}

void print_servmsg(const struct servmsg *msg)
{
    printf("servmsg: type=%c, numq=%lu,\n",
           msg->type,
           msg->numq);
    // print questions
    for (int i = 0; i < msg->numq; i++)
    {
        printf("Q%d: %ls\n", i, msg->questions[i].q);
        for (int j = 0; j < OPTIONNUM; j++)
        {
            printf("\t%d: %ls\n", j, msg->questions[i].option[j]);
        }
    }
    printf("player=%c, scorechange=%d, correct=%c\n",
           msg->player,
           msg->scorechange,
           msg->correct);
    printf("numplayer=%lu,\n", msg->numplayer);
    // resultdata
    for (int i = 0; i < msg->numplayer; i++) {
        printf("\tplayer %d: score=%d, coin=%d\n",
            i, 
            msg->resultdata[i].score,
            msg->resultdata[i].coin);
    }
}

char inttochar(int num)
{
    return num + '0';
}

void print_hex(char *byte, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        if (i % 8 == 0 && i != 0)
            putchar('\n');
        printf("%02hhX ", byte[i]);
    }
    putchar('\n');
}
