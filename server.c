#include <stdio.h>

#define CDT_IMPL
#include "cdt.h"

#define ENET_IMPLEMENTATION
#include "enet.h"

#define MAX_CLIENTS 2

int main() {
    if (enet_initialize () != 0) {
        printf("An error occurred while initializing ENet.\n");
        return 1;
    }

    ENetAddress address = {
        .host = ENET_HOST_ANY,
        .port = 7777,
    };

    ENetHost *server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

    if (!server) {
        printf("An error occurred while trying to create an ENet server host.\n");
        return 1;
    }

    printf("Started a server...\n");

    ENetEvent event;
    for (;;) {
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
                event.peer->data = "Client information";
            } break;

            case ENET_EVENT_TYPE_RECEIVE: {
                printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                        event.packet->dataLength,
                        event.packet->data,
                        event.peer->data,
                        event.channelID);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);
            } break;

            case ENET_EVENT_TYPE_DISCONNECT: {
                printf("%s disconnected.\n", event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            } break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                printf("%s disconnected due to timeout.\n", event.peer->data);
                /* Reset the peer's client information. */
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
