#include <locale.h>
#include <assert.h>
#include "common.h"

void test_climsg_ser()
{
    char buf[MAXLINE];
    time_t t = time(NULL);
    struct climsg msg = {ANSWER, '1', t};
    ssize_t pktlen = serialize_climsg(&msg, buf, sizeof(buf));
    assert(pktlen > 0);
    char *cur = buf;
    assert(*cur == ANSWER);
    cur++;
    assert(*cur == '1');
    cur++;
    assert(*(time_t *)(cur) == t);
    cur += sizeof(t);
    assert(*cur == PKTEND);

    char menuopt = '2';
    struct climsg msg2 = {MENU, .menuopt = menuopt};
    pktlen = serialize_climsg(&msg2, buf, sizeof(buf));
    assert(pktlen > 0);
    cur = buf;
    assert(*cur == MENU);
    cur++;
    assert(*cur == menuopt);
    cur++;
    assert(*cur == PKTEND);

    struct climsg msg3 = {CLI_LOGIN, .id = "testid", .pw = "testpw"};
    pktlen = serialize_climsg(&msg3, buf, sizeof(buf));
    assert(pktlen > 0);
    cur = buf;
    char *tmp = strchr(cur, ',');
    assert(*tmp == ',');
    *tmp = 0; // null term
    assert(0 == strcmp(&msg3.id, cur));
    cur = tmp + 1;
    tmp = strchr(cur, PKTEND);
    *tmp = 0;
    assert(0 == strcmp(&msg3.pw, cur));
    // cur += strlen(cur);
    puts("test climsg serialize pass");
}

void test_climsg_des()
{
    char buf[MAXLINE];
    // answers
    time_t t = time(NULL);
    struct climsg msg = {ANSWER, '1', t};
    ssize_t pktlen = serialize_climsg(&msg, buf, sizeof(buf));
    struct climsg out;
    int ret = deserialize_climsg(&out, buf, pktlen);
    assert(ret >= 0);
    assert(out.type == msg.type);
    assert(out.ans == msg.ans);
    assert(out.anstime == msg.anstime);
    // menu options
    char m = '1';
    struct climsg msg2 = {MENU, .menuopt = m};
    pktlen = serialize_climsg(&msg2, buf, sizeof(buf));
    ret = deserialize_climsg(&out, buf, pktlen);
    assert(ret >= 0);
    assert(out.type == msg2.type);
    assert(out.menuopt == msg2.menuopt);
    // login
    struct climsg msg3 = {CLI_LOGIN, .id = "testid", .pw = "testpw"};
    pktlen = serialize_climsg(&msg3, buf, sizeof(buf));
    ret = deserialize_climsg(&out, buf, pktlen);
    assert(ret >= 0);
    assert(out.type == msg3.type);
    assert(0 == strcmp(out.id, msg3.id));
    assert(0 == strcmp(out.pw, msg3.pw));

    puts("test climsg deserialize pass");
}

void test_servmsg_ser()
{
    char buf[MAXLINE];
    char *cur = buf;
    ssize_t pktlen;
    // init2p
    struct question q[2] = {
        {L"問題一", {L"一", L"二", L"三", L"四"}},
        {L"問題二", {L"五", L"六", L"七", L"八"}}};
    struct servmsg msg = {
        INIT_2P,
        '0',
        "testoppid",
        2,
        q};
    pktlen = serialize_servmsg(&msg, buf, sizeof(buf));
    assert(pktlen >= 0);
    assert(*cur == msg.type);
    cur++;
    assert(*cur == msg.assigned);
    cur++;
    strchr(cur, ',');
    assert(*(size_t *)cur == msg.numq);
    cur += sizeof(msg.numq);
    // questions
    for (int i = 0; i < msg.numq; i++)
    {
        struct question *currq = &msg.questions[i];
        size_t n = wcslen(currq->q);
        assert(0 == wmemcmp((wchar_t *)cur, currq->q, n));
        cur += (n + 1) * sizeof(wchar_t);
        for (int j = 0; j < OPTIONNUM; j++)
        {
            n = wcslen(currq->option[j]);
            assert(0 == wmemcmp((wchar_t *)cur, currq->option[j], n));
            cur += (n + 1) * sizeof(wchar_t);
        }
    }
    cur -= sizeof(wchar_t);
    assert(*cur == PKTEND);
    // eval_ans
    struct servmsg msg2 = {
        EVAL_ANS,
        .player = '1',
        .scorechange = 55555,
        .correct = '0'};
    pktlen = serialize_servmsg(&msg2, buf, sizeof(buf));
    assert(pktlen >= 0);
    cur = buf;
    assert(*cur == msg2.type);
    cur++;
    assert(*cur == msg2.player);
    cur++;
    assert(*(int *)cur == msg2.scorechange);
    cur += sizeof(msg2.scorechange);
    assert(*cur == msg2.correct);
    cur++;
    assert(*cur == PKTEND);
    // game_result
    struct player_result res[2] = {{100, 200}, {300, 400}};
    struct servmsg msg3 = {
        GAME_RESULT,
        .numplayer = 2,
        .resultdata = res};
    pktlen = serialize_servmsg(&msg3, buf, sizeof(buf));
    assert(pktlen >= 0);
    cur = buf;
    assert(*cur == msg3.type);
    cur++;
    // player_result
    for (int i = 0; i < msg3.numplayer; i++)
    {
        assert(*(int *)cur == msg3.resultdata[i].score);
        cur += sizeof(msg3.resultdata[i].score);
        assert(*(int *)cur == msg3.resultdata[i].coin);
        cur += sizeof(msg3.resultdata[i].coin);
    }
    assert(*cur == PKTEND);
    puts("test servmsg serialize pass");
}

void test_servmsg_des()
{
    char buf[MAXLINE];
    ssize_t pktlen;
    int ret;
    struct question outq[MAXNUMQ];
    struct player_result resbuf[MAXPLAYER];
    struct servmsg out = {
        .questions = outq,
        .resultdata = resbuf};
    // questions
    struct question q[2] = {
        {L"問題一", {L"一", L"二", L"三", L"四"}},
        {L"問題二", {L"五", L"六", L"七", L"八"}}};
    struct servmsg msg = {
        INIT_2P,
        2,
        q};
    pktlen = serialize_servmsg(&msg, buf, sizeof(buf));
    ret = deserialize_servmsg(&out, buf, pktlen);
    assert(ret >= 0);
    assert(out.numq == msg.numq);
    for (int i = 0; i < msg.numq; i++)
    {
        size_t n = wcslen(msg.questions[i].q);
        assert(0 == wmemcmp(out.questions[i].q, msg.questions[i].q, n));
        for (int j = 0; j < OPTIONNUM; j++)
        {
            size_t n = wcslen(msg.questions[i].option[j]);
            assert(0 == wmemcmp(out.questions[i].option[j], msg.questions[i].option[j], n));
        }
    }
    // eval_ans
    msg = (struct servmsg){
        EVAL_ANS,
        .player = '0',
        .scorechange = 435621,
        .correct = '1'};
    pktlen = serialize_servmsg(&msg, buf, sizeof(buf));
    ret = deserialize_servmsg(&out, buf, pktlen);
    assert(ret >= 0);
    assert(out.player == msg.player);
    assert(out.scorechange == msg.scorechange);
    assert(out.correct == msg.correct);
    // game result
    struct player_result res[2] = {
        {100, 200},
        {300, 400}};
    msg = (struct servmsg){
        GAME_RESULT,
        .numplayer = 2,
        .resultdata = res};
    pktlen = serialize_servmsg(&msg, buf, sizeof(buf));
    ret = deserialize_servmsg(&out, buf, pktlen);
    assert(ret >= 0);
    assert(out.numplayer == msg.numplayer);
    for (int i = 0; i < msg.numplayer; i++)
    {
        assert(out.resultdata[i].score == msg.resultdata[i].score);
        assert(out.resultdata[i].coin == msg.resultdata[i].coin);
    }

    puts("test servmsg deserialize pass");
}

int main()
{
    // to print Chinese char
    setlocale(LC_ALL, "");
    // climsg
    // test_climsg_ser();
    // test_climsg_des();
    // // servmsg
    // test_servmsg_ser();
    // test_servmsg_des();
    struct question outq[MAXNUMQ];
    struct player_result resbuf[MAXPLAYER];
    struct servmsg out = {
        .questions = outq,
        .resultdata = resbuf};
    // questions
    struct question q[2] = {
        {L"問題一", {L"一", L"二", L"三", L"四"}},
        {L"問題二", {L"五", L"六", L"七", L"八"}}};
    struct servmsg msg = {
        INIT_2P,
        2,
        q};
    cpy_servmsg(&out, &msg);
    print_servmsg(&out);
}