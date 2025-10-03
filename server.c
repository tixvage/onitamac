#include <stdio.h>

#define CDT_IMPL
#include "cdt.h"

#define ENET_IMPLEMENTATION
#include "enet.h"

#include "common.h"

#define MAX_CLIENTS 2

typedef struct Client {
    u32 id;
    ENetPeer *peer;
} Client;

// Ultimate unique id generator
u32 id_counter = 5;
u32 gen_id(void) {
    return ++id_counter;
}

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
            } break;

            case ENET_EVENT_TYPE_RECEIVE: {
                info("a packet of length %lu was received from client(%lu) on channel %u",
                        event.packet->dataLength,
                        find_client_peer(event.peer)->id,
                        event.channelID);
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
