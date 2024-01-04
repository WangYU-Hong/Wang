#include <stdio.h>
#include <wchar.h>
#include <time.h>
#include "unp.h"
#include "common.h"

int deserialize_climsg(struct climsg *msg, const void *buf, size_t pktlen)
{
    char *bytebuf = (char *)buf;
    if (pktlen <= 0)
        goto fail;

    memset(msg, 0, sizeof(struct climsg));

    char type = bytebuf[0];
    int msgtype = atoi(&type);
    switch (msgtype)
    {
    case ANSWER: // ans, anstime
        if (pktlen != 11)
            goto fail;
        char ans = bytebuf[1];
        time_t *anstime = (time_t *)&bytebuf[2];

        msg->ans = atoi(&ans);
        msg->anstime = *anstime;
        break;

    case MENU:
        if (pktlen != 3)
            goto fail;
        char menuopt = bytebuf[1];
        msg->menuopt = atoi(&menuopt);
        break;

    default:
        goto fail;
    }
    msg->type = msgtype;
    return 0;

fail:
    printf("%s error: invalid pkt\n", __func__);
    return -1;
}

void print_climsg(const struct climsg* msg)
{
    printf("type=%d, ans=%d, anstime=%ld, menuopt=%d", msg->type, msg->ans, msg->anstime, msg->menuopt);
}

int serialize_climsg(const struct climsg *msg, void *buf, size_t buflen)
{
    int buflenerr = 0, climsgerr = 0;
    int pktlen;
    switch (msg->type)
    {
    case ANSWER:
        pktlen = 11;
        if (buflen < pktlen) {
            buflenerr = 1;
            goto fail;
        }
        memset(buf, 0, pktlen);
        snprintf(buf, buflen, "%c%c%s", 
            inttochar(msg->type), 
            inttochar(msg->ans), 
            (char*) &msg->anstime
        );
        break;
    
    case MENU:
        pktlen = 3;
        if (buflen < pktlen) {
            buflenerr = 1;
            goto fail;
        }

        snprintf(buf, buflen, "%c%c", 
            inttochar(msg->type), 
            inttochar(msg->menuopt)
        );
        break;

    default:
        climsgerr = 1;
        goto fail;
        break;
    }
    return pktlen;

fail:
    printf("%s error, ", __func__);
    if (buflenerr) {
        puts("buflen too short");
    }
    else if (climsgerr) {
        puts("invalid climsg");
    }
    return -1;
}

char inttochar(int num) {
    return num + '0';
}

void print_hex(char* byte, size_t len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", byte[i]);
    }
    putchar('\n');
}
