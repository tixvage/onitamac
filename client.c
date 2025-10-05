#include <raylib.h>

#define CDT_IMPL
#include "cdt.h"

#define ENET_IMPLEMENTATION
#include "enet.h"

#include "common.h"

// START: Macros & Constants
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define BOARD_WIDTH   400
#define BOARD_HEIGHT  400

#define BORDER_THICKNESS 2.f

#define BOARD_SIZE 5
#define BOARD_CENTER 12

#define MOVE_TEXTURE_SIZE 100.f
#define MOVE_TEXTURE_RECT_SIZE (MOVE_TEXTURE_SIZE / BOARD_SIZE)

#define PIECE_WIDTH (BOARD_WIDTH  / BOARD_SIZE)
#define PIECE_HEIGHT (BOARD_HEIGHT / BOARD_SIZE)

#define HOST_ADDRESS "127.0.0.1"
#define PORT_ADDRESS 7777

// 25 bit + (first) 7 bits must be skipped
//                        i like to put `,` at the end
//                                       |
//                                       |
//                                       V
#define DEFINE_TABLE(_1, _2, _3, _4, _5, ...) \
                        ((0b ## _1 ## U) << 20) | \
                        ((0b ## _2 ## U) << 15) | \
                        ((0b ## _3 ## U) << 10) | \
                        ((0b ## _4 ## U) <<  5) | \
                        ((0b ## _5 ## U) <<  0)

#define hex(_x) { \
    .r = (_x >> 24) & 0xFF, \
    .g = (_x >> 16) & 0xFF, \
    .b = (_x >>  8) & 0xFF, \
    .a = (_x >>  0) & 0xFF, \
}

const Rectangle board_rect = {
    (WINDOW_WIDTH  - BOARD_WIDTH)  / 2,
    (WINDOW_HEIGHT - BOARD_HEIGHT) / 2 - 50,
    BOARD_WIDTH, BOARD_HEIGHT,
};

Texture2D soldier_texture;
Texture2D master_texture;
// END: Macros & Constants

// START: Colors
typedef enum Palette_Colors {
    Palette_BACKGROUND,
    Palette_BOARD,
    Palette_BOARD_BORDER,
    Palette_MM,
    Palette_MS,
    Palette_EM,
    Palette_ES,
    Palette_HIGHLIGHT,
    Palette_CHOSEN,
    Palette_INVALID_MOVE,
    Palette_VALID_MOVE,
    Palette_EAT_MOVE,
    Palette_ENEMY_FROM,
    Palette_ENEMY_TO,

    Palette_Count,
} Palette_Colors;

const Color PALETTE[Palette_Count] = {
    [Palette_BACKGROUND]   = hex(0x0d0e14ff),
    [Palette_BOARD]        = hex(0x252933ff),
    [Palette_BOARD_BORDER] = hex(0x0f151dff),
    [Palette_MM]           = hex(0x597d7cff),
    [Palette_MS]           = hex(0x386775ff),
    [Palette_EM]           = hex(0x205a4eff),
    [Palette_ES]           = hex(0x193d31ff),
    [Palette_HIGHLIGHT]    = hex(0x333f4cff),
    [Palette_CHOSEN]       = hex(0x404556ff),
    [Palette_INVALID_MOVE] = hex(0x60515cff),
    [Palette_VALID_MOVE]   = hex(0x504556ff),
    [Palette_EAT_MOVE]     = hex(0x75505aff),
    [Palette_ENEMY_FROM]   = hex(0x205a4eff),
    [Palette_ENEMY_TO]     = hex(0x193d31ff),
};
// END: Colors

// START: Table Bit Management
static inline u8 xy_to_index(u8 x, u8 y) {
    return (y * BOARD_SIZE + x);
}

static inline u32 shift_to(u8 x, u8 y) {
    return 1 << ((BOARD_SIZE*BOARD_SIZE) - (y * BOARD_SIZE + x + 1));
}

u8 get_bit(u32 table, u8 x, u8 y) {
    u32 shifted = shift_to(x, y) & table;
    return shifted > 0;
}

u8 get_bit_index(u32 table, u8 index) {
    return get_bit(table, index % BOARD_SIZE, index / BOARD_SIZE);
}

void enable_bit(u32 *table, u8 x, u8 y) {
    *table |= shift_to(x, y);
}

void enable_bit_index(u32 *table, u8 index) {
    enable_bit(table, index % BOARD_SIZE, index / BOARD_SIZE);
}

void disable_bit(u32 *table, u8 x, u8 y) {
    *table &= ~shift_to(x, y);
}

void disable_bit_index(u32 *table, u8 index) {
    disable_bit(table, index % BOARD_SIZE, index / BOARD_SIZE);
}
// END: Table Bit Management

// START: Pieces
typedef enum Piece_Type {
    Piece_MM = 0,
    Piece_MS,
    Piece_EM,
    Piece_ES,

    Piece_Count,
} Piece_Type;

Color get_piece_color(u8 piece) {
    switch (piece) {
    case Piece_MM: return PALETTE[Palette_MM];
    case Piece_MS: return PALETTE[Palette_MS];
    case Piece_EM: return PALETTE[Palette_EM];
    case Piece_ES: return PALETTE[Palette_ES];
    }

    return PALETTE[Palette_BOARD];
}
// END: Pieces

// START: Movements
typedef enum Move_Type {
    Move_TIGER,
    Move_DRAGON,
    Move_FROG,
    Move_RABBIT,
    Move_CRAB,
    Move_ELEPHANT,
    Move_GOOSE,
    Move_ROOSTER,
    Move_MONKEY,
    Move_MANTIS,
    Move_HORSE,
    Move_OX,
    Move_CRANE,
    Move_BOAR,
    Move_EEL,
    Move_COBRA,

    Move_Count,
} Move_Type;

//https://boardgamegeek.com/image/2999035/onitama
const u32 MOVE_TEMPLATES[Move_Count] = {
    [Move_TIGER] = DEFINE_TABLE(
        00100,
        00000,
        00000,
        00100,
        00000,
    ),
    [Move_DRAGON] = DEFINE_TABLE(
        00000,
        10001,
        00000,
        01010,
        00000,
    ),
    [Move_FROG] = DEFINE_TABLE(
        00000,
        01000,
        10000,
        00010,
        00000,
    ),
    [Move_RABBIT] = DEFINE_TABLE(
        00000,
        00010,
        00001,
        01000,
        00000,
    ),
    [Move_CRAB] = DEFINE_TABLE(
        00000,
        00100,
        10001,
        00000,
        00000,
    ),
    [Move_ELEPHANT] = DEFINE_TABLE(
        00000,
        01010,
        01010,
        00000,
        00000,
    ),
    [Move_GOOSE] = DEFINE_TABLE(
        00000,
        01000,
        01010,
        00010,
        00000,
    ),
    [Move_ROOSTER] = DEFINE_TABLE(
        00000,
        00010,
        01010,
        01000,
        00000,
    ),
    [Move_MONKEY] = DEFINE_TABLE(
        00000,
        01010,
        00000,
        01010,
        00000,
    ),
    [Move_MANTIS] = DEFINE_TABLE(
        00000,
        01010,
        00000,
        00100,
        00000,
    ),
    [Move_HORSE] = DEFINE_TABLE(
        00000,
        00100,
        01000,
        00100,
        00000,
    ),
    [Move_OX] = DEFINE_TABLE(
        00000,
        00100,
        00010,
        00100,
        00000,
    ),
    [Move_CRANE] = DEFINE_TABLE(
        00000,
        00100,
        00000,
        01010,
        00000,
    ),
    [Move_BOAR] = DEFINE_TABLE(
        00000,
        00100,
        01010,
        00000,
        00000,
    ),
    [Move_EEL] = DEFINE_TABLE(
        00000,
        01000,
        00010,
        01000,
        00000,
    ),
    [Move_COBRA] = DEFINE_TABLE(
        00000,
        00010,
        01000,
        00010,
        00000,
    ),
};

static u32 MOVES[Move_Count][BOARD_SIZE*BOARD_SIZE] = { 0 };
static Texture2D MOVE_TEXTURES[Move_Count] = { 0 };

void calculate_moves(void) {
    usize center = 2;

    for (usize i = 0; i < Move_Count; i++) {
        u32 template = MOVE_TEMPLATES[i];
        for (usize py = 0; py < BOARD_SIZE; py++) {
            for (usize px = 0; px < BOARD_SIZE; px++) {
                u32 result = 0;
                for (usize my = 0; my < BOARD_SIZE; my++) {
                    for (usize mx = 0; mx < BOARD_SIZE; mx++) {
                        if (get_bit(template, mx, my) > 0) {
                            i32 board_x = px + (mx - center);
                            i32 board_y = py + (my - center);
                            if (board_x >= 0 && board_x < BOARD_SIZE &&
                                    board_y >= 0 && board_y < BOARD_SIZE) {
                                enable_bit(&result, board_x, board_y);
                            }
                        }
                    }
                }
                MOVES[i][py * BOARD_SIZE + px] = result;
            }
        }
    }
}

void init_move_textures(void) {
    usize center = 2;
    for (usize i = 0; i < Move_Count; i++) {
        u32 template = MOVE_TEMPLATES[i];
        Image img = GenImageColor(MOVE_TEXTURE_SIZE, MOVE_TEXTURE_SIZE, PALETTE[Palette_BOARD]);
        for (usize y = 0; y < BOARD_SIZE; y++) {
            for (usize x = 0; x < BOARD_SIZE; x++) {
                Rectangle rect = {
                    x * MOVE_TEXTURE_RECT_SIZE,
                    y * MOVE_TEXTURE_RECT_SIZE,
                    MOVE_TEXTURE_RECT_SIZE,
                    MOVE_TEXTURE_RECT_SIZE,
                };
                ImageDrawRectangleLines(&img, rect, 1.f, PALETTE[Palette_BOARD_BORDER]);

                if (y == center && x == center) {
                    ImageDrawCircle(&img, rect.x + MOVE_TEXTURE_RECT_SIZE / 2.f, rect.y + MOVE_TEXTURE_RECT_SIZE / 2.f, MOVE_TEXTURE_RECT_SIZE * 0.3f, PALETTE[Palette_MM]);
                    continue;
                }
                if (get_bit(template, x, y) == 0) {
                    ImageDrawCircleLines(&img, rect.x + MOVE_TEXTURE_RECT_SIZE / 2.f, rect.y + MOVE_TEXTURE_RECT_SIZE / 2.f, MOVE_TEXTURE_RECT_SIZE * 0.3f, PALETTE[Palette_BOARD_BORDER]);
                } else {
                    ImageDrawCircle(&img, rect.x + MOVE_TEXTURE_RECT_SIZE / 2.f, rect.y + MOVE_TEXTURE_RECT_SIZE / 2.f, MOVE_TEXTURE_RECT_SIZE * 0.3f, PALETTE[Palette_EM]);
                }
            }
        }
        MOVE_TEXTURES[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }
}
// END: Movements

// START: Various Functions
static inline Vector2 get_piece_pos(usize x, usize y) {
    return (Vector2){
        board_rect.x + x * PIECE_WIDTH,
        board_rect.y + y * PIECE_HEIGHT,
    };
}

static inline Rectangle get_piece_rect(usize x, usize y) {
    Vector2 p = get_piece_pos(x, y);

    Rectangle r = {
        p.x, p.y,
        PIECE_WIDTH, PIECE_HEIGHT,
    };

    return r;
}

static inline Rectangle get_piece_rect_highlight(usize x, usize y) {
    Rectangle piece_rect = get_piece_rect(x, y);

    piece_rect.x += 1;
    piece_rect.y += 1;
    piece_rect.width -= 2;
    piece_rect.height -= 2;

    return piece_rect;
}

void custom_raylib_log(int msgType, const char *text, va_list args) {
    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(stderr, "[%s]", timeStr);

    switch (msgType)
    {
        case LOG_INFO: fprintf(stderr, "[INFO](raylib): "); break;
        case LOG_ERROR: fprintf(stderr, "[ERROR](raylib): "); break;
        case LOG_WARNING: fprintf(stderr, "[WARN](raylib): "); break;
        case LOG_DEBUG: fprintf(stderr, "[DEBUG](raylib): "); break;
        default: break;
    }

    vprintf(text, args);
    printf("\n");
}
// END: Various Functions

// START: Global State
// all the global data that will change during game
static struct G {
    u32 PIECES[Piece_Count];
    i32 highlighted_index;
    i32 selected_index;
    usize selected_group;
    usize selected_move;
    bool control_mode_mouse;
    ENetHost *client;
    ENetPeer *peer;
    u32 id;
    bool turn;
    u32 match_id;
    i8 e_moved_to;
    i8 e_moved_from;
} G = {
    .PIECES = {
        [Piece_MM] = 
            DEFINE_TABLE(
                00000,
                00000,
                00000,
                00000,
                00100,
            ),
        [Piece_EM] =
            DEFINE_TABLE(
                00100,
                00000,
                00000,
                00000,
                00000,
            ),
        [Piece_MS] = 
            DEFINE_TABLE(
                00000,
                00000,
                00000,
                00000,
                11011,
            ),
        [Piece_ES] =
            DEFINE_TABLE(
                11011,
                00000,
                00000,
                00000,
                00000,
            ),
    },
    .highlighted_index = 12,
    .selected_index = -1,
    .selected_group = Piece_Count,
    .selected_move = Move_ROOSTER,
    .control_mode_mouse = false,
    .client   = NULL,
    .peer     = NULL,
    .turn     = false,
    .match_id = 0,
    .e_moved_to   = -1,
    .e_moved_from = -1,
};

void select_piece(usize group, i32 index) {
    G.selected_group = group;
    G.selected_index = index;
}

void unselect_piece(void) {
    G.selected_group = Piece_Count;
    G.selected_index = -1;
}

void draw_piece(usize x, usize y, Texture2D t, Color c) {
    Rectangle source = {
        0, 0,
        t.width,
        t.height,
    };
    usize index = y * BOARD_SIZE + x;
    const f32 scale_factor = G.highlighted_index == cast(i32) index ? 4.f : 3.f;
    f32 pos_x = board_rect.x + x * PIECE_WIDTH;
    f32 pos_y = board_rect.y + y * PIECE_HEIGHT;
    Rectangle dest = {
        pos_x + (PIECE_WIDTH  - t.width  * scale_factor) / 2,
        pos_y + (PIECE_HEIGHT - t.height * scale_factor) / 2,
        t.width  * scale_factor,
        t.height * scale_factor,
    };
    Vector2 origin = {
        0, 0,
    };
    DrawTexturePro(t, source, dest, origin, 0, c);
}

void handle_selection(usize index) {
    if (!G.turn) return;
    if (G.selected_index != -1) {
        u32 moves = MOVES[G.selected_move][G.selected_index];
        u32 friends = G.PIECES[Piece_MS] | G.PIECES[Piece_MM];

        moves &= ~friends;
        if (get_bit_index(moves, G.highlighted_index) > 0) {
            Move move = {
                .match_id  = G.match_id,
                .m = {
                    .group = G.selected_group,
                    .from  = G.selected_index,
                    .to    = index,
                },
            };
            enable_bit_index(&G.PIECES[G.selected_group], index);
            for (usize i = Piece_EM; i <= Piece_ES; i++) {
                if (get_bit_index(G.PIECES[i], index) > 0) {
                    move.e.eat   = true;
                    move.e.group = i;
                    move.e.index = index;
                    disable_bit_index(&G.PIECES[i], index);
                }
            }
            disable_bit_index(&G.PIECES[G.selected_group], G.selected_index);
            G.turn = false;
            // Reset previous enemy move it is pointless to still being able to see after our turn finishes
            G.e_moved_from = -1;
            G.e_moved_to   = -1;
            Packet p = {
                .type = Packet_MOVE,
                .move = move,
            };
            ENetPacket* packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(G.peer, 0, packet);
            info("turn finished");
        }
        unselect_piece();
    } else {
        for (usize i = Piece_MM; i <= Piece_MS; i++) {
            if (get_bit_index(G.PIECES[i], index) == 0) continue;
            select_piece(i, index);
            break;
        }
    }
}

void new_match(Match match) {
    G.turn = match.c[0] == G.id;
    G.match_id = match.id;
}
// END: Global State

int main(void) {
    if (enet_initialize() != 0) {
        panic("an error occurred while initializing enet");
    }
    info("enet initialized successfully");
    G.client = enet_host_create(NULL, 1, 2, 0, 0);
    if (!G.client) {
        panic("an error occurred while trying to create an ENet client host");
    }
    info("client host created successfully");
    ENetAddress address = { 0 };
    address.port = PORT_ADDRESS;
    enet_address_set_host(&address, HOST_ADDRESS);

    ENetEvent event = { 0 };
    G.peer = enet_host_connect(G.client, &address, 2, 0);
    if (!G.peer) {
        panic("no available peers for initiating an enet connection");
    }
    info("available peers for enet connection");
    if (enet_host_service(G.client, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT) {
        info("connection to "HOST_ADDRESS" succeeded");
    } else {
        enet_peer_reset(G.peer);
        panic("connection to "HOST_ADDRESS" failed");
    }

    calculate_moves();
    info("movevements calculated successfully");

    SetTraceLogCallback(custom_raylib_log);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "game");
    init_move_textures();
    info("movevement textures created successfully");
    SetTargetFPS(60);

    soldier_texture = LoadTexture("soldier.png");
    master_texture = LoadTexture("master.png");

    while (!WindowShouldClose()) {
        // Inputs
        {
            i32 d = IsKeyPressed(KEY_X) - IsKeyPressed(KEY_Z);
            i32 as_i = G.selected_move + d;
            if (as_i >= Move_Count) {
                as_i = 0;
            } else if (as_i < 0) {
                as_i = Move_Count - 1;
            }
            G.selected_move = as_i;
        }
        if (IsKeyPressed(KEY_F)) {
            G.control_mode_mouse = !G.control_mode_mouse;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            G.control_mode_mouse = true;
        }
        if (G.control_mode_mouse) {
            G.highlighted_index = -1;
            Vector2 mouse_pos = GetMousePosition();
            if (CheckCollisionPointRec(mouse_pos, board_rect)) {
                f32 dx = mouse_pos.x - board_rect.x;
                f32 dy = mouse_pos.y - board_rect.y;
                i32 x = dx / PIECE_WIDTH;
                i32 y = dy / PIECE_HEIGHT;
                usize index = y * BOARD_SIZE + x;
                G.highlighted_index = index;

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    handle_selection(index);
                }
            } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                unselect_piece();
            }
        // My mouse is broken right now :(
        } else {
            i32 dir_y = IsKeyPressed(KEY_DOWN)  - IsKeyPressed(KEY_UP);
            i32 dir_x = IsKeyPressed(KEY_RIGHT) - IsKeyPressed(KEY_LEFT);
            G.highlighted_index = clamp(G.highlighted_index + dir_y * BOARD_SIZE + dir_x, 0, BOARD_SIZE*BOARD_SIZE - 1);

            if (IsKeyPressed(KEY_BACKSPACE)) {
                unselect_piece();
            }

            if (IsKeyPressed(KEY_ENTER)) {
                handle_selection(G.highlighted_index);
            }
        }

        // Networking
        if (enet_host_service(G.client, &event, 0) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                info("a packet of length %lu was received on channel %u",
                        event.packet->dataLength,
                        event.channelID);
                Packet p = *cast(Packet *)event.packet->data;
                switch (p.type) {
                case Packet_GIVE_ID: {
                    G.id = p.give_id;
                    SetWindowTitle(TextFormat("client(%lu)", G.id));
                } break;
                case Packet_SAY_HELLO: {
                } break;
                case Packet_NEW_MATCH: {
                    info("match found");
                    new_match(p.new_match);
                } break;
                case Packet_MOVE: {
                    G.turn = true;
                    // Flip move
                    Move move = p.move;
                    usize moved_group = move.m.group + 2;
                    u8    moved_from  = (BOARD_SIZE*BOARD_SIZE-1) - move.m.from;
                    u8    moved_to    = (BOARD_SIZE*BOARD_SIZE-1) - move.m.to;

                    info("move group(%lu) from index(%u) to index(%u)", moved_group, moved_from, moved_to);

                    G.e_moved_from = moved_from;
                    G.e_moved_to   = moved_to;
                    disable_bit_index(&G.PIECES[moved_group], moved_from);
                    enable_bit_index (&G.PIECES[moved_group], moved_to);
                    // TODO: eating
                    if (move.e.eat) {
                        usize eaten_group = move.e.group - 2;
                        u8 index = (BOARD_SIZE*BOARD_SIZE-1) - move.e.index;
                        info("eat group(%lu) in index(%u)", eaten_group, index);
                        disable_bit_index(&G.PIECES[eaten_group], index);
                    }
                    info("turn started");
                }
                case Packet_Count: {
                } break;
                }
                enet_packet_destroy (event.packet);
            } break;
            default: {
                info("unhandled enet event(%d)", event.type);
            } break;
            }
        }

        // Rendering
        BeginDrawing();
        ClearBackground(PALETTE[Palette_BACKGROUND]);

        DrawRectangleRec(board_rect, PALETTE[Palette_BOARD]);
        DrawRectangleLinesEx(board_rect, BORDER_THICKNESS, PALETTE[Palette_BOARD_BORDER]);

        // Draw board
        for (usize y = 0; y < BOARD_SIZE; y++) {
            for (usize x = 0; x < BOARD_SIZE; x++) {
                usize index = y * BOARD_SIZE + x;
                Rectangle piece_rect = get_piece_rect(x, y);
                Rectangle highlight_rect = get_piece_rect_highlight(x, y);
                // TODO: Put one more pixel line around the whole board
                DrawRectangleLinesEx(piece_rect, BORDER_THICKNESS / 2.f, PALETTE[Palette_BOARD_BORDER]);
                if (G.selected_index == cast(i32) index) {
                    DrawRectangleRec(highlight_rect, PALETTE[Palette_CHOSEN]);
                }
                if (G.e_moved_from == cast(i8) index) {
                    DrawRectangleRec(highlight_rect, PALETTE[Palette_ENEMY_FROM]);
                }
                if (G.e_moved_to == cast(i8) index) {
                    DrawRectangleRec(highlight_rect, PALETTE[Palette_ENEMY_TO]);
                }

                if (G.selected_index != -1) {
                    u32 moves = MOVES[G.selected_move][G.selected_index];
                    u32 friends = G.PIECES[Piece_MS] | G.PIECES[Piece_MM];
                    u32 enemies = G.PIECES[Piece_ES] | G.PIECES[Piece_EM];

                    u32 valid_moves = moves & ~friends;
                    if (get_bit(moves, x, y) > 0) {
                        Rectangle rect = get_piece_rect_highlight(x, y);
                        Color c = PALETTE[Palette_INVALID_MOVE];
                        if (get_bit(valid_moves, x, y) > 0) {
                            c = PALETTE[Palette_VALID_MOVE];
                        }
                        if (get_bit(enemies, x, y) > 0) {
                            c = PALETTE[Palette_EAT_MOVE];
                        }
                        DrawRectangleRec(rect, c);
                    }
                }
                if (G.highlighted_index == cast(i32) index) {
                    DrawRectangleRec(highlight_rect, PALETTE[Palette_HIGHLIGHT]);
                }

                for (usize i = 0; i < Piece_Count; i++) {
                    u32 p = G.PIECES[i];
                    u8 bit = get_bit(p, x, y);
                    if (bit == 0) continue;
                    Color c = get_piece_color(i);
                    Texture2D t = soldier_texture;
                    if (i == Piece_MM || i == Piece_EM) t = master_texture;
                    draw_piece(x, y, t, c);
                }
            }
        }

        Rectangle z_rect = board_rect;
        z_rect.y     += z_rect.height + 35;
        z_rect.width  = 70;
        z_rect.height = 70;

        DrawRectangleRec(z_rect, PALETTE[Palette_BOARD]);
        DrawRectangleLinesEx(z_rect, 4.f, PALETTE[Palette_BOARD_BORDER]);
        // TODO: hardcoded ahh
        DrawText("Z", z_rect.x + 14, z_rect.y + 4, 70, PALETTE[Palette_BACKGROUND]);

        Rectangle x_rect = board_rect;
        x_rect.x     += x_rect.width - 70;
        x_rect.y     += x_rect.height + 35;
        x_rect.width  = 70;
        x_rect.height = 70;

        DrawRectangleRec(x_rect, PALETTE[Palette_BOARD]);
        DrawRectangleLinesEx(x_rect, 4.f, PALETTE[Palette_BOARD_BORDER]);
        // TODO: hardcoded ahh
        DrawText("X", x_rect.x + 14, x_rect.y + 4, 70, PALETTE[Palette_BACKGROUND]);

        DrawTexture(MOVE_TEXTURES[G.selected_move], (WINDOW_WIDTH - MOVE_TEXTURE_SIZE) / 2.f, WINDOW_HEIGHT - (MOVE_TEXTURE_SIZE + 30), WHITE);

        EndDrawing();
    }

    CloseWindow();

    enet_host_service(G.client, &event, 1000);
    enet_peer_disconnect(G.peer, 0);

    bool disconnected = false;
    while (enet_host_service(G.client, &event, 1000) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            enet_packet_destroy(event.packet);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
            info("disconnection succeeded");
            disconnected = true;
        } break;
        default: break;
        }
    }

    if (!disconnected) {
        enet_peer_reset(G.peer);
    }

    enet_host_destroy(G.client);
    enet_deinitialize();

    info("enet deinitialized");

    return 0;
}
