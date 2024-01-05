#include <locale.h>
#include "common.h"
#define MAXLEN 4096

int main() {
    setlocale(LC_ALL, "");
    // test serialize
    struct climsg msg = {ANSWER, '1', 12345678, '2'};
    char buf[MAXLEN];
    int pktlen = serialize_climsg(&msg, buf, sizeof(buf));
    print_hex(buf, pktlen);

    // deserialize
    struct climsg outmsg;
    deserialize_climsg(&outmsg, buf, pktlen);
    print_climsg(&outmsg);

    // test servmsg serialize
    struct question q[2] = {
        {L"問題一", {L"一", L"二", L"三", L"四"}},
        {L"問題二", {L"一", L"二", L"三", L"四"}}
    };
    struct player_result res[2] = {{100, 200},{300, 400}};
    struct servmsg servmsg = {GAME_RESULT, 2, q, '0', 987654321, '0', 2, res};
    pktlen = serialize_servmsg(&servmsg, buf, sizeof(buf));
    print_hex(buf, pktlen);
    print_servmsg(&servmsg);
    // test servmsg deserialize

}