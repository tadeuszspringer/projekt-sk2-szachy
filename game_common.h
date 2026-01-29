#ifndef GAME_COMMON_H
#define GAME_COMMON_H

struct Allowed
{
    int key;
    int allowed[8][8];
};
struct GameSession {
    int whiteSocket;
    int blackSocket;
};

#endif