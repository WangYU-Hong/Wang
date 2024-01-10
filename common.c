#include <stdio.h>
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
        *(char *)cur = PKTEND;
        cur += nbytes;
        break;

    case MENU:
        pktlen = 3;
        if (buflen < pktlen)
        {
            buflenerr = 1;
            goto fail;
        }

        snprintf(buf, buflen, "%c%c%c",
                 msg->type,
                 msg->menuopt,
                 PKTEND);
        break;

    case CLI_LOGIN:
    case CLI_REGISTER:
        pktlen = strlen(msg->id) + strlen(msg->menuopt) + 2;
        if (buflen < pktlen)
        {
            buflenerr = 1;
            goto fail;
        }
        snprintf(
            buf,
            buflen,
            "%c%c,%c%c",
            msg->type,
            msg->id,
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

    case CLI_LOGIN:
    case CLI_REGISTER:
        char *cur = bytebuf + 1;
        int i = 0, success = 0;
        for (; i < pktlen - 1; i++)
        {
            // found delim
            if (cur[i] == ',')
            {
                strncpy(msg->id, cur, i);
                cur += i + 1;
                break;
            }
            // found PKTEND
            if (cur[i] == PKTEND)
            {
                goto fail;
            }
        }
        for (; i < pktlen - 1; i++)
        {
            if (cur[i] != DELIM)
                continue;
            strncpy(msg->pw, cur, i);
            success = 1;
        }
        if (!success)
        {
            goto fail;
        }
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
    printf("type=%c, ans=%c, anstime=%ld, menuopt=%c, id=%s, pw=%s\n",
           msg->type,
           msg->ans,
           msg->anstime,
           msg->menuopt,
           msg->id,
           msg->pw);
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
    const wchar_t delim[] = {DELIM, L'\0'};
    for (int i = 0; i < numq; i++)
    {
        const struct question *currq = &questions[i];

        // q
        n = mvcurwcpy(&cur, currq->q, &buflen);
        if (n < 0)
            goto fail;
        pktlen += n;
        // ","
        n = mvcurwcpy(&cur, delim, &buflen);
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
            n = mvcurwcpy(&cur, delim, &buflen);
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
    const wchar_t *cur = buf;
    int nwchar = -1, cpyoptions = 0, i = 0, readq = 0;
    while (readq < numq && ((void *)cur - buf) < pktlen)
    {
        nwchar++;
        if (cur[nwchar] != DELIM && *(char *)(cur + nwchar) != PKTEND)
            continue;

        // q
        if (!cpyoptions)
        {
            wmemcpy(questions[readq].q, cur, nwchar);
            questions[readq].q[nwchar] = L'\0'; // null terminate
            cpyoptions = 1;
        }
        else
        { // cpy options
            wmemcpy(questions[readq].option[i], cur, nwchar);
            questions[readq].option[i][nwchar] = L'\0';
            if (++i >= OPTIONNUM)
            { // question complete
                i = 0;
                cpyoptions = 0;
                readq++;
            }
        }
        cur += nwchar + 1; // skip delim
        nwchar = 0;
    }
    // if (cpyoptions || readq == 0) {
    if (readq < numq) // incomplete questions
    {
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
    char *cur = buf;
    switch (msg->type)
    {
    case INIT_2P: // numq, qs
        if (buflen < 2)
            goto fail;
        type = msg->type;
        n = sizeof(type);
        memcpy(buf, &type, n);
        pktlen += n;
        cur += n;

        n = sizeof(msg->assigned);
        memcpy(cur, &msg->assigned, n);
        pktlen += n;
        cur += n;

        n = strlen(msg->oppid);
        memcpy(cur, &msg->oppid, n + 1); // with null
        pktlen += n + 1;
        cur += n + 1;

        n = sizeof(msg->numq);
        memcpy(cur, &msg->numq, n);
        pktlen += n;
        cur += n;

        n = serialize_question(msg->questions, msg->numq, cur, buflen - pktlen);
        if (n < 0)
            goto fail;
        pktlen += n;
        cur += n;

        if (buflen < pktlen + 1)
            goto fail;
        *cur = PKTEND;
        pktlen += 1;
        break;

    case EVAL_ANS:
        if (buflen < 6 + sizeof(int))
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

        n = sizeof(msg->ans);
        memcpy(cur, &msg->ans, n);
        cur += n;

        n = sizeof(msg->oppans);
        memcpy(cur, &msg->oppans, n);
        cur += n;

        n = 1;
        *cur = PKTEND;
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
        *cur = PKTEND;
        cur += n;
        pktlen = cur - (char *)buf;
        break;

    case SERV_LOGIN:
    case SERV_REGISTER:
        if (buflen < 3)
            goto fail;
        snprintf(
            buf,
            buflen,
            "%c%c%c",
            msg->type,
            msg->success,
            PKTEND);
        pktlen = 3;
        break;

    default:
        goto fail;
        break;
    }
    return pktlen;
fail:
    printf("%s error, not enough buffer\n", __func__);
    return -1;
}

int deserialize_servmsg(struct servmsg *msg, const void *buf, size_t pktlen) // TODO
{
    int ret, nplayer = 0;
    const char *cur = buf;
    if (pktlen <= 0)
        goto fail;

    char type = cur[0];
    cur++;
    switch (type)
    {
    case INIT_2P:

        // assigned
        msg->assigned = *cur;
        cur += sizeof(msg->assigned);
        // oppid
        strncpy(msg->oppid, cur, sizeof(msg->oppid));
        cur += strlen(msg->oppid) + 1; // include null
        // numq
        msg->numq = *(size_t *)cur;
        cur += sizeof(msg->numq);
        // question
        ret = deserialize_question(msg->questions, msg->numq, cur, pktlen - ((void *)cur - buf));
        if (ret < 0)
            goto fail;

        break;

    case EVAL_ANS:
        // player
        msg->player = *cur;
        cur++;
        // scorechange
        msg->scorechange = *(int *)cur;
        cur += sizeof(msg->scorechange);
        // correct
        msg->correct = *cur;
        cur += sizeof(msg->correct);
        // ans
        msg->ans = *cur;
        cur += sizeof(msg->ans);
        // oppans
        msg->oppans = *cur;
        cur += sizeof(msg->oppans);

        break;

    case GAME_RESULT:
        // calculate numplayer, player result

        while (*cur != PKTEND && ((void *)cur - buf) < pktlen)
        {
            msg->resultdata[nplayer].score = *(int *)cur;
            cur += sizeof(msg->resultdata->score);
            msg->resultdata[nplayer].coin = *(int *)cur;
            cur += sizeof(msg->resultdata->coin);
            nplayer++;
        }
        msg->numplayer = nplayer;
        break;

    case SERV_LOGIN:
    case SERV_REGISTER:
        msg->success = *cur;
        cur += sizeof(msg->success);

    default:
        goto fail;
        break;
    }
    return 0;

fail:
    printf("%s error: invalid pkt\n", __func__);
    return -1;
}

void print_servmsg(const struct servmsg *msg)
{
    printf("servmsg: type=%c, assigned=%c, oppid=%s, numq=%lu,\n",
           msg->type,
           msg->assigned,
           msg->oppid,
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
    printf("player=%c, scorechange=%d, correct=%c, ans=%c, oppand=%c\n",
           msg->player,
           msg->scorechange,
           msg->correct,
           msg->ans,
           msg->oppans);
    printf("numplayer=%lu,\n", msg->numplayer);
    // resultdata
    for (int i = 0; i < msg->numplayer; i++)
    {
        printf("\tplayer %d: score=%d, coin=%d\n",
               i,
               msg->resultdata[i].score,
               msg->resultdata[i].coin);
    }
    printf("success=%c\n", msg->success);
}

void cpy_servmsg(struct servmsg *dst, struct servmsg *src)
{
    dst->type = src->type;
    dst->assigned = src->assigned;
    strncpy(dst->oppid, src->oppid, sizeof(dst->oppid));
    dst->numq = src->numq;
    // questions
    for (int i = 0; i < src->numq; i++)
    {
        dst->questions[i] = src->questions[i];
    }
    dst->player = src->player;
    dst->scorechange = src->scorechange;
    dst->correct = src->correct;
    dst->ans = src->ans;
    dst->oppans = src->oppans;

    dst->numplayer = src->numplayer;
    for (int i = 0; i < src->numplayer; i++)
    {
        dst->resultdata[i] = src->resultdata[i];
    }
    dst->success = src->success;
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
