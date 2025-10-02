#include <raylib.h>

#define CDT_IMPL
#include "cdt.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define BOARD_WIDTH   400
#define BOARD_HEIGHT  400

#define BORDER_THICKNESS 2.f

// 25 bit + 7 bits must be skipped
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

typedef enum Palette_Colors {
    Palette_BACKGROUND,
    Palette_BOARD,
    Palette_BOARD_HIGHLIGHT1,
    Palette_BOARD_HIGHLIGHT2,
    Palette_BOARD_HIGHLIGHT3,
    Palette_BOARD_BORDER,
    Palette_MM,
    Palette_MS,
    Palette_EM,
    Palette_ES,

    Palette_Count,
} Palette_Colors;

const Color PALETTE[Palette_Count] = {
    [Palette_BACKGROUND]       = hex(0x0d0e14ff),
    [Palette_BOARD]            = hex(0x252933ff),
    [Palette_BOARD_HIGHLIGHT1] = hex(0x333f4cff),
    [Palette_BOARD_HIGHLIGHT2] = hex(0x404556ff),
    [Palette_BOARD_HIGHLIGHT3] = hex(0x60515cff),
    [Palette_BOARD_BORDER]     = hex(0x0f151dff),
    [Palette_MM]               = hex(0x597d7cff),
    [Palette_MS]               = hex(0x386775ff),
    [Palette_EM]               = hex(0x20504eff),
    [Palette_ES]               = hex(0x193d31ff),
};

#define BOARD_SIZE 5
#define BOARD_CENTER 12

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

// all the global data that will change during game
static struct G {
    u32 PIECES[Piece_Count];
    i32 highlighted_index;
    i32 selected_index;
    usize selected_group;
    usize selected_move;
    bool control_mode_mouse;
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
};

void select_piece(usize group, i32 index) {
    G.selected_group = group;
    G.selected_index = index;
}

void unselect_piece(void) {
    G.selected_group = Piece_Count;
    G.selected_index = -1;
}

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

void precalculate_moves(void) {
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

Texture2D pawn_texture;

const Rectangle board_rect = {
    (WINDOW_WIDTH - BOARD_WIDTH) / 2,
    (WINDOW_HEIGHT - BOARD_HEIGHT) / 2,
    BOARD_WIDTH, BOARD_HEIGHT,
};

const f32 PIECE_WIDTH = 
    (BOARD_WIDTH  / BOARD_SIZE);
const f32 PIECE_HEIGHT = 
    (BOARD_HEIGHT / BOARD_SIZE);

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


void draw_piece(usize x, usize y, Color c) {
    Rectangle source = {
        0, 0,
        pawn_texture.width,
        pawn_texture.height,
    };
    usize index = y * BOARD_SIZE + x;
    const f32 scale_factor = G.highlighted_index == cast(i32) index ? 4.f : 3.f;
    f32 pos_x = board_rect.x + x * PIECE_WIDTH;
    f32 pos_y = board_rect.y + y * PIECE_HEIGHT;
    Rectangle dest = {
        pos_x + (PIECE_WIDTH - pawn_texture.width * scale_factor) / 2,
        pos_y + (PIECE_HEIGHT - pawn_texture.height * scale_factor) / 2,
        pawn_texture.width  * scale_factor,
        pawn_texture.height * scale_factor,
    };
    Vector2 origin = {
        0, 0,
    };
    DrawTexturePro(pawn_texture, source, dest, origin, 0, c);
}

void draw_piece_group(Piece_Type type) {
    u32 p = G.PIECES[type];
    Color c = get_piece_color(type);
    for (usize y = 0; y < BOARD_SIZE; y++) {
        for (usize x = 0; x < BOARD_SIZE; x++) {
            u8 bit = get_bit(p, x, y);
            if (bit == 0) continue;
            draw_piece(x, y, c);
        }
    }
}

void handle_selection(usize index) {
    if (G.selected_index != -1) {
        u32 moves = MOVES[G.selected_move][G.selected_index];
        u32 friends = G.PIECES[Piece_MS] | G.PIECES[Piece_MM];

        moves &= ~friends;
        if (get_bit_index(moves, G.highlighted_index) > 0) {
            enable_bit_index(&G.PIECES[G.selected_group], index);
            for (usize i = Piece_EM; i <= Piece_ES; i++) {
                if (get_bit_index(G.PIECES[i], index) > 0) {
                    disable_bit_index(&G.PIECES[i], index);
                }
            }
            disable_bit_index(&G.PIECES[G.selected_group], G.selected_index);
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

int main(void) {
    precalculate_moves();

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "game");
    SetTargetFPS(60);

    pawn_texture = LoadTexture("pawn.png");

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F)) {
            G.control_mode_mouse = !G.control_mode_mouse;
        }
        if (G.control_mode_mouse) {
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
            i32 temp = G.highlighted_index;
            temp += dir_y * BOARD_SIZE + dir_x;
            G.highlighted_index = clamp(temp, 0, BOARD_SIZE*BOARD_SIZE - 1);

            if (IsKeyPressed(KEY_BACKSPACE)) {
                unselect_piece();
            }

            if (IsKeyPressed(KEY_ENTER)) {
                handle_selection(G.highlighted_index);
            }
        }

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
                    DrawRectangleRec(highlight_rect, PALETTE[Palette_BOARD_HIGHLIGHT2]);
                }

                if (G.selected_index != -1) {
                    u32 moves = MOVES[G.selected_move][G.selected_index];
                    u32 friends = G.PIECES[Piece_MS] | G.PIECES[Piece_MM];

                    u32 valid_moves = moves & ~friends;
                    if (get_bit(moves, x, y) > 0) {
                        Rectangle rect = get_piece_rect_highlight(x, y);
                        DrawRectangleRec(rect, PALETTE[get_bit(valid_moves, x, y) > 0 ? Palette_BOARD_HIGHLIGHT2 : Palette_BOARD_HIGHLIGHT3]);
                    }
                }
                if (G.highlighted_index == cast(i32) index) {
                    DrawRectangleRec(highlight_rect, PALETTE[Palette_BOARD_HIGHLIGHT1]);
                }

                for (usize i = 0; i < Piece_Count; i++) {
                    u32 p = G.PIECES[i];
                    Color c = get_piece_color(i);
                    u8 bit = get_bit(p, x, y);
                    if (bit == 0) continue;
                    draw_piece(x, y, c);
                }
            }
        }

        EndDrawing();

        if (G.control_mode_mouse) {
            G.highlighted_index = -1;
        }
    }

    CloseWindow();
    return 0;
}
