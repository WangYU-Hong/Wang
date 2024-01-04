#include "unp.h"
#include <wchar.h>
#include <time.h>

enum climsgtype {ANSWER = 1, MENU = 2};

/*If a member is invalid, that member will be zero.
Example: type=ANSWER, menuopt=0*/ 
struct climsg
{
    enum climsgtype type;
    // Type 1: Answers
    int ans;
    time_t anstime;
    // Type 2: Menu Options
    int menuopt;
};

void print_climsg(const struct climsg* msg);
/*Serialize msg into buf.
Buf will terminate with '\0'.
Return pkt len on success, -1 on failure*/
int serialize_climsg(const struct climsg* msg, void* buf, size_t buflen);
/*Deserialize buf into climsg.
Buf should terminate with '\0'.
Return -1 on failure.*/
int deserialize_climsg(struct climsg* msg, const void* buf, size_t pktlen);

/*Convert 0-9 into char.*/
char inttochar(int num);
/*Print the byte array as hex*/
void print_hex(char* buf, size_t len);