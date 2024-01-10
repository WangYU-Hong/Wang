#pragma once
#include <wchar.h>
#include <time.h>

// for unp.h
typedef unsigned int u_int;
typedef unsigned char u_char;
#include "unp.h"

enum climsgtype
{
    ANSWER = '1',
    MENU = '2',
    CLI_LOGIN = '3',
    CLI_REGISTER = '4'
};

#define LOGIN_MAXLEN 32

/*If a member is invalid, that member will be zero.
Example: `type=ANSWER, menuopt=0`*/
struct climsg
{
    enum climsgtype type;
    // Type 1: Answers
    char ans; // '0' means timeout
    time_t anstime;
    // Type 2: Menu Options
    char menuopt;
    // Type 3/4: Login
    // should be null-terminated
    char id[LOGIN_MAXLEN];
    char pw[LOGIN_MAXLEN];
};

void print_climsg(const struct climsg *msg);
/*Serialize `msg` into `buf`.
`buf` will terminate with '\\n'.
Return `pktlen` on success, `-1` on failure*/
ssize_t serialize_climsg(const struct climsg *msg, void *buf, size_t buflen);
/*Deserialize buf into climsg.
Buf should terminate with '\\n'.
Return -1 on failure.*/
int deserialize_climsg(struct climsg *msg, const void *buf, size_t pktlen);

enum servmsgtype
{
    INIT_2P = '1',
    EVAL_ANS = '2',
    GAME_RESULT = '3',
    SERV_LOGIN = '4',
    SERV_REGISTER = '5'
};

#define OPTIONNUM 4
#define Q_MAXLEN 256
#define MAXNUMQ 32
#define MAXPLAYER 10

struct question
{
    wchar_t q[Q_MAXLEN];
    wchar_t option[OPTIONNUM][Q_MAXLEN];
    char ans; // not used in client
};

/*Return actual bytes copied, or -1 on error*/
ssize_t mvcurwcpy(wchar_t **cur, const wchar_t *src, size_t *buflen);

#define DELIM L','
#define PKTEND '\n'

ssize_t serialize_question(const struct question *questions, size_t numq, void *buf, size_t buflen);
int deserialize_question(struct question *questions, size_t numq, const void *buf, size_t pktlen);

struct player_result
{
    int score;
    int coin;
};

// You should prepare buffers for struct `question` and `player_result`.
struct servmsg
{
    enum servmsgtype type;
    // Type 1: 2p game
    char assigned; // '0' to '9'
    char oppid[LOGIN_MAXLEN];
    size_t numq; // number of questions
    struct question *questions;
    // Type 2: client answer response
    char player; // '0' to '9'
    int scorechange;
    char correct; // '0' or '1'
    char ans;     // true answer
    char oppans;  // opponent answer
    // Type 3: game result
    size_t numplayer;
    struct player_result *resultdata;
    // Type 4/5: login/register
    char success; // '0' or '1'
};

void cpy_servmsg(struct servmsg *dst, struct servmsg *src);
void print_servmsg(const struct servmsg *msg);
/* Serialize msg into buf.
buf will terminate with '\\n'.
Return pktlen on success, -1 on error.*/
ssize_t serialize_servmsg(const struct servmsg *msg, void *buf, size_t buflen);
/*Deserialize buf in to msg.
Return -1 on error.*/
int deserialize_servmsg(struct servmsg *msg, const void *buf, size_t pktlen);

/*Convert 0-9 into char.*/
char inttochar(int num);
/*Print the byte array as hex*/
void print_hex(char *buf, size_t len);

// my errors
#define SERVCLOSED -1
#define TIMEOUT -2
#define OTHERERROR -99
#define OPPDC -3