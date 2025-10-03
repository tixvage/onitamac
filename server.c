#include <stdio.h>

#define CDT_IMPL
#include "cdt.h"

#define ENET_IMPLEMENTATION
#include "enet.h"

#include "common.h"

#define MAX_CLIENTS 2
#define MAX_MATCHS (MAX_CLIENTS/2)

// Ultimate unique id generator
u32 id_counter = 5;
u32 gen_id(void) {
    return ++id_counter;
}

Match matchs[MAX_MATCHS] = { 0 };
usize matchs_count = 0;

Match *new_match(void) {
    return &matchs[matchs_count++];
}

Match *find_match_id(u32 id) {
    for (usize i = 0; i < matchs_count; i++) {
        if (matchs[i].id == id) return &matchs[i];
    }

    return NULL;
}

typedef struct Client {
    u32 id;
    ENetPeer *peer;
} Client;

Client clients[MAX_CLIENTS] = { 0 };
usize  clients_count = 0;

Client *new_client(void) {
    return &clients[clients_count++];
}

Client *find_client_id(u32 id) {
    for (usize i = 0; i < clients_count; i++) {
        if (clients[i].id == id) return &clients[i];
    }

    return NULL;
}

Client *find_client_peer(ENetPeer *peer) {
    for (usize i = 0; i < clients_count; i++) {
        if (clients[i].peer == peer) return &clients[i];
    }

    return NULL;
}

int main() {
    if (enet_initialize () != 0) {
        panic("an error occurred while initializing enet");
    }
    info("enet initialized successfully");

    ENetAddress address = {
        .host = ENET_HOST_ANY,
        .port = 7777,
    };

    ENetHost *server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

    if (!server) {
        panic("an error occurred while trying to create an ENet server host");
    }
    info("server host created successfully");

    info("started a server");

    ENetEvent event;
    for (;;) {
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                Client *c = new_client();
                c->id   = gen_id();
                c->peer = event.peer;

                Packet p = {
                    .type = Packet_GIVE_ID,
                    .give_id = c->id,
                };
                info("new connection client(%lu)", c->id);
                ENetPacket* packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(event.peer, 0, packet);

                if (clients_count == 2) {
                    info("enough players for match");
                    Client c1 = clients[clients_count - 2];
                    Client c2 = clients[clients_count - 1];
                    Match *match = new_match();
                    match->id    = gen_id();
                    match->c[0]  = c1.id;
                    match->c[1]  = c2.id;
                    info("match(%lu) created", match->id);
                    Packet p = {
                        .type = Packet_NEW_MATCH,
                        .new_match = *match,
                    };
                    ENetPacket* packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(c1.peer, 0, packet);
                    enet_peer_send(c2.peer, 0, packet);
                }
            } break;

            case ENET_EVENT_TYPE_RECEIVE: {
                info("a packet of length %lu was received from client(%lu) on channel %u",
                        event.packet->dataLength,
                        find_client_peer(event.peer)->id,
                        event.channelID);
                Client *sender = find_client_peer(event.peer);
                Packet p = *cast(Packet *)event.packet->data;
                switch (p.type) {
                case Packet_MOVE: {
                    Match *match = find_match_id(p.move.match_id);
                    info("move in match(%lu) from client(%lu) > group(%lu) from(%u) to(%u)", match->id, sender->id, p.move.m.group, p.move.m.from, p.move.m.to);
                    u32 target_id = match->c[0];
                    if (sender->id == target_id) {
                        target_id = match->c[1];
                    }
                    Client *opp = find_client_id(target_id);
                    // Maybe we need to flip move data in there
                    Move m = p.move;
                    Packet p = {
                        .type = Packet_MOVE,
                        .move = m,
                    };
                    ENetPacket* packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(opp->peer, 0, packet);
                } break;
                case Packet_NEW_MATCH:
                case Packet_SAY_HELLO:
                case Packet_GIVE_ID:
                case Packet_Count: {
                    info("invalid packet");
                } break;
                }
                enet_packet_destroy (event.packet);
            } break;

            case ENET_EVENT_TYPE_DISCONNECT: {
                info("client(%lu) disconnected", find_client_peer(event.peer)->id);
                // TODO: modify client array
                event.peer->data = NULL;
            } break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                info("client(%lu) disconnected due to timeout", find_client_peer(event.peer)->id);
                event.peer->data = NULL;
            } break;

            case ENET_EVENT_TYPE_NONE: {
            } break;
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}
