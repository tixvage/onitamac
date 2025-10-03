#ifndef _COMMON_H
#define _COMMON_H

typedef enum Packet_Type {
    Packet_GIVE_ID = 0,
    Packet_SAY_HELLO,

    Packet_Count,
} Packet_Type;

typedef struct Packet {
    Packet_Type type;
    union {
        u32 give_id;
        void *say_hello;
    };
} Packet;

#endif
