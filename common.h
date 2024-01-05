#include "unp.h"
#include <wchar.h>
#include <time.h>

enum climsgtype {
    ANSWER = '1', 
    MENU = '2'
};

/*If a member is invalid, that member will be zero.
Example: type=ANSWER, menuopt=0*/ 
struct climsg
{
    enum climsgtype type;
    // Type 1: Answers
    char ans;
    time_t anstime;
    // Type 2: Menu Options
    char menuopt;
};

void print_climsg(const struct climsg* msg);
/*Serialize msg into buf.
Buf will terminate with '\\n'.
Return pkt len on success, -1 on failure*/
ssize_t serialize_climsg(const struct climsg* msg, void* buf, size_t buflen);
/*Deserialize buf into climsg.
Buf should terminate with '\\n'.
Return -1 on failure.*/
int deserialize_climsg(struct climsg* msg, const void* buf, size_t pktlen);

enum servmsgtype {
    INIT_2P = '1', 
    EVAL_ANS='2', 
    GAME_RESULT='3'
};

#define OPTIONNUM 4
#define Q_MAXLEN 256

struct question {
    wchar_t q[Q_MAXLEN];
    wchar_t option[OPTIONNUM][Q_MAXLEN];
};

/*Return actual bytes copied, or -1 on error*/
ssize_t mvcurwcpy(wchar_t** cur, const wchar_t* src, size_t* buflen);

#define DELIM L','
#define PKTEND '\n'

ssize_t serialize_question(const struct question* questions, size_t numq, void* buf, size_t buflen);
int deserialize_question(struct question* questions, size_t numq, const void* buf, size_t pktlen);

struct player_result
{
    int score;
    int coin;
};

struct servmsg {
    enum servmsgtype type;
    // Type 1: 2p game
    size_t numq; // number of questions
    struct question* questions;
    // Type 2: client answer response
    char player; // '0' to '9'
    int scorechange;
    char correct; // '0' or '1'
    // Type 3: game result
    size_t numplayer;
    struct player_result* resultdata;
};

void print_servmsg(const struct servmsg* msg);
/* Serialize msg into buf.
buf will terminate with '\\n'.
Return pkt len on success, -1 on error.*/
ssize_t serialize_servmsg(const struct servmsg* msg, void* buf, size_t buflen);
/*Deserialize buf in to msg.
Return -1 on error.*/
int deserialize_servmsg(struct servmsg* msg, const void* buf, size_t pktlen);

/*Convert 0-9 into char.*/
char inttochar(int num);
/*Print the byte array as hex*/
void print_hex(char* buf, size_t len);