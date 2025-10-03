#ifndef _COMMON_H
#define _COMMON_H

#include "cdt.h"

typedef enum Packet_Type {
    Packet_GIVE_ID = 0,
    Packet_SAY_HELLO,
    Packet_NEW_MATCH,
    Packet_MOVE,

    Packet_Count,
} Packet_Type;

typedef struct Match {
    u32 id;
    u32 c[2];
} Match;

typedef struct Move {
    u32 match_id;
    struct {
        usize group;
        u8    from;
        u8    to;
    } m;
    struct {
        bool  eat;
        usize group;
        u8    index;
    } e;
} Move;

typedef struct Packet {
    Packet_Type type;
    union {
        u32   give_id;
        void *say_hello;
        Match new_match;
        Move  move;
    };
} Packet;

#endif
