#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_LENGTH 50

int PLAYABLE_WIDTH = 12;
int PLAYABLE_HEIGHT = 16;

#define FONT_SIZE 40
#define SCORE_COLOR RAYWHITE
#define PAUSE_COLOR ORANGE

#define CENTRE_X (TABLE_WIDTH / 2 - SHAPE_FRAME_LENGTH / 2)
#define START_X CENTRE_X
#define START_Y WALL_THICKNESS
#define DEFAULT_FALLTIME 0.6f

#define WINDOW_WIDTH (BLOCK_LENGTH * PLAYABLE_WIDTH)
#define WINDOW_HEIGHT (BLOCK_LENGTH * PLAYABLE_HEIGHT)
#define FPS 120

#define WALL_THICKNESS 2
#define TABLE_WIDTH (PLAYABLE_WIDTH + WALL_THICKNESS * 2)
#define TABLE_HEIGHT (PLAYABLE_HEIGHT + WALL_THICKNESS * 2)

#define SHAPE_FRAME_LENGTH 4

#define UINT64_HIGH_1_BIT (0x1ULL << 63)
#define UINT64_HIGH_4_BIT (0xFULL << 60)

#define UINT16_HIGH_1_BIT (0x1U << 15)
#define UINT16_HIGH_4_BIT (0xFU << 12)

#define EMPTY_LINE_MASK ((UINT64_MAX >> (PLAYABLE_WIDTH + WALL_THICKNESS)) | (UINT64_MAX << (64 - WALL_THICKNESS)))
#define FULL_LINE_MASK (~EMPTY_LINE_MASK)

#define ROTATE_SHAPE_ID(id, r) (((id) & ~3) | (((id) + (r) + 4) & 3))

typedef uint64_t table_t;
typedef uint16_t shape_t;
typedef int8_t shapeid_t;
typedef int8_t offset_t;
typedef int8_t pos_t;
typedef uint32_t score_t;

// NULL, I, O, T, S, Z, J, L
const shape_t shapes[32] = {
    0, 0, 0, 0,
    0b0000111100000000, 0b0010001000100010, 0b0000000011110000, 0b0100010001000100,
    0b1100110000000000, 0b1100110000000000, 0b1100110000000000, 0b1100110000000000,
    0b1110010000000000, 0b0010011000100000, 0b0000010011100000, 0b1000110010000000,
    0b0110110000000000, 0b0100011000100000, 0b0000011011000000, 0b1000110001000000,
    0b1100011000000000, 0b0010011001000000, 0b0000110001100000, 0b0100110010000000,
    0b1000111000000000, 0b0110010001000000, 0b0000111000100000, 0b0100010011000000,
    0b1110100000000000, 0b0110001000100000, 0b0000001011100000, 0b1000100011000000,
};

typedef struct {
    table_t* table;
    shapeid_t shapeId;
    pos_t shapeX, shapeY;
    score_t scoreCurrent, scoreHighest;
    float fallTime;
    bool pause;
} Game;

void PrintTable(const table_t table[]) {
    for (int y = 0; y < TABLE_HEIGHT; ++y) {
        printf("[y = %2d]", y);
        for (int x = 0; x < 64; ++x) {
            if (!(x % 4)) putchar(' ');
            table_t bit = table[y] & (UINT64_HIGH_1_BIT >> x);
            putchar(bit ? '#' : '.');
        }
        putchar('\n');
    }
    putchar('\n');
}

void InitTable(table_t table[]) {
    memset(table, 0, sizeof(table_t) * TABLE_HEIGHT);
    for (int y = 0; y < WALL_THICKNESS; ++y) {
        table[y] = UINT64_MAX;
        table[TABLE_HEIGHT - 1 - y] = UINT64_MAX;
    }
    for (int y = WALL_THICKNESS; y < TABLE_HEIGHT - WALL_THICKNESS; ++y) {
        table[y] = (UINT64_MAX >> (PLAYABLE_WIDTH + WALL_THICKNESS)) | (UINT64_MAX << (64 - WALL_THICKNESS));
    }
    PrintTable(table);
}

void DrawTable(const table_t table[]) {
    for (int y = 0; y < PLAYABLE_HEIGHT; ++y) {
        for (int x = 0; x < PLAYABLE_WIDTH; ++x) {
            table_t bit = table[y + WALL_THICKNESS] & (UINT64_HIGH_1_BIT >> (x + WALL_THICKNESS));
            Color color = bit ? WHITE : BLANK;
            DrawRectangle(x * BLOCK_LENGTH, y * BLOCK_LENGTH, BLOCK_LENGTH, BLOCK_LENGTH, color);
        }
    }
}

void DrawShape(const Game* game) {
    shape_t shape = shapes[game->shapeId];
    pos_t shapeX = game->shapeX;
    pos_t shapeY = game->shapeY;
    for (int row = 0; row < SHAPE_FRAME_LENGTH; ++row) {
        for (int col = 0; col < SHAPE_FRAME_LENGTH; ++col) {
            shape_t lineOfShapeFrame = (shape << (row * SHAPE_FRAME_LENGTH)) & UINT16_HIGH_4_BIT;
            shape_t bit = lineOfShapeFrame & (UINT16_HIGH_1_BIT >> col);
//            printf("%d\n", bit);
            Color color = bit ? WHITE : BLANK;
            DrawRectangle((shapeX + col - WALL_THICKNESS) * BLOCK_LENGTH, (shapeY + row - WALL_THICKNESS) * BLOCK_LENGTH, BLOCK_LENGTH, BLOCK_LENGTH, color);
        }
    }
//    printf("-----------------------\n");
}

bool IsOverlap(const table_t* table, shapeid_t shapeId, pos_t shapeX, pos_t shapeY) {
    table_t shape = (table_t)shapes[shapeId] << 48;
    for (int row = 0; row < SHAPE_FRAME_LENGTH; ++row) {
        table_t lineOfShapeFrame = ((shape << (row * SHAPE_FRAME_LENGTH)) & UINT64_HIGH_4_BIT) >> shapeX;
        table_t lineOfTable = table[shapeY + row];
        table_t isOverlap = lineOfShapeFrame & lineOfTable;
        if (isOverlap) return true;
    }
    return false;
}

bool CanMove(const Game* game, offset_t offsetX, offset_t offsetY, offset_t offsetRotate) {
    table_t* table = game->table;
    shapeid_t shapeId = offsetRotate ? ROTATE_SHAPE_ID(game->shapeId, offsetRotate) : game->shapeId;
    pos_t shapeX = game->shapeX;
    pos_t shapeY = game->shapeY;
    pos_t targetX = shapeX + offsetX;
    pos_t targetY = shapeY + offsetY;
    if (targetX < 0 || targetX > TABLE_WIDTH) return false;
    if (targetY < 0 || targetY > TABLE_HEIGHT) return false;
    if (IsOverlap(table, shapeId, targetX, targetY)) return false;
    return true;
}

bool Update(Game* game, offset_t offsetX, offset_t offsetY, offset_t offsetRotate) {
    if (CanMove(game, offsetX, offsetY, offsetRotate)) {
        game->shapeX += offsetX;
        game->shapeY += offsetY;
        game->shapeId = ROTATE_SHAPE_ID(game->shapeId, offsetRotate);
        return true;
    }
    return false;
}

void CommitShape(Game* game) {
    table_t shape = (table_t)shapes[game->shapeId] << 48;
    table_t* table = game->table;
    pos_t shapeX = game->shapeX;
    pos_t shapeY = game->shapeY;
    for (int row = 0; row < SHAPE_FRAME_LENGTH; ++row) {
        table_t lineOfShapeFrame = ((shape << (row * SHAPE_FRAME_LENGTH)) & UINT64_HIGH_4_BIT) >> shapeX;
        table_t* lineOfTable = &table[shapeY + row];
        *lineOfTable |= lineOfShapeFrame;
    }
}

void GameOver(Game* game);

void NewShape(Game* game) {
    game->shapeId = GetRandomValue(0, 1000) % 28 + 4;
    game->shapeX = START_X;
    game->shapeY = START_Y;
    if (IsOverlap(game->table, game->shapeId, game->shapeX, game->shapeY)) GameOver(game);
}

void GameOver(Game* game) {
    score_t scoreCurrent = game->scoreCurrent;
    score_t scoreHighest = game->scoreHighest;
    game->scoreHighest = scoreCurrent > scoreHighest ? scoreCurrent : scoreHighest;
    game->scoreCurrent = 0;
    InitTable(game->table);
    NewShape(game);
}

void CheckLines(Game* game) {
    table_t* table = game->table;
    for (int y = TABLE_HEIGHT - WALL_THICKNESS - 1; y >= WALL_THICKNESS; --y) {
        if ((table[y] & FULL_LINE_MASK) == FULL_LINE_MASK) {
            game->scoreCurrent += 10;
            for (int t = y; t >= WALL_THICKNESS + 1; --t) {
                table[t] = table[t - 1];
            }
            y = TABLE_HEIGHT - WALL_THICKNESS;
        }
    }
}

void NaturalFall(Game* game) {
    if (!game->pause && !Update(game, 0, 1, 0)) {
        CommitShape(game);
        CheckLines(game);
        NewShape(game);
    }
}

void InitGame(Game* game) {
    game->scoreCurrent = 0;
    game->scoreHighest = 0;
    game->shapeX = START_X;
    game->shapeY = START_Y;
    game->fallTime = DEFAULT_FALLTIME;
    game->shapeId = GetRandomValue(0, 1000) % 28 + 4;
    game->pause = false;
    InitTable(game->table);
}

void HandleInput(Game* game) {
    int key = GetKeyPressed();
    if (!game->pause) {
        if (key == KEY_H || key == KEY_A || key == KEY_LEFT) Update(game, -1, 0, 0);
        if (key == KEY_L || key == KEY_D || key == KEY_RIGHT) Update(game, 1, 0, 0);
        if (key == KEY_J || key == KEY_S || key == KEY_DOWN) Update(game, 0, 0, -1);
        if (key == KEY_K || key == KEY_W || key == KEY_UP) Update(game, 0, 0, 1);
    }
    if (key == KEY_R) GameOver(game);
    if (key == KEY_P || key == KEY_ESCAPE) game->pause = !game->pause;

    if (IsKeyDown(KEY_SEMICOLON) || IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) game->fallTime = 0.08f;
    else game->fallTime = DEFAULT_FALLTIME;

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyDown(KEY_Q)) {
        CloseWindow();
        exit(0);
    }
}

int main(int argc, char* argv[])
{
    if (argc == 3) {
        int w = atoi(argv[1]);
        int h = atoi(argv[2]);
        if (w < 4 || w > 60 || h < 4 || h > 60) {
            printf("Invalid arguments.\nUseage: %s [width:int[4,60]] [height:int[4,60]]\n", argv[0]);
            return 1;
        }
        PLAYABLE_WIDTH = w;
        PLAYABLE_HEIGHT = h;
    } else if (argc != 1) {
        printf("Invalid arguments.\nUseage: %s [width:int[4,60]] [height:int[4,60]]\n", argv[0]);
        return 1;
    }

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris");
    SetTargetFPS(FPS);
    SetExitKey(KEY_NULL);

    float timer = 0.f;
    table_t table[TABLE_HEIGHT];
    Game game = { .table = table };
    InitGame(&game);

    while (!WindowShouldClose()) {
        HandleInput(&game);

        BeginDrawing();
        ClearBackground(BLACK);

        timer += GetFrameTime(); 
        if (timer >= game.fallTime) {
            NaturalFall(&game);
            timer -= game.fallTime;
        }

        DrawTable(table);
        DrawShape(&game);
        DrawText(TextFormat("%06d", game.scoreHighest), FONT_SIZE / 2, FONT_SIZE / 2, FONT_SIZE, SCORE_COLOR);
        DrawText(TextFormat("%06d", game.scoreCurrent), FONT_SIZE / 2, FONT_SIZE / 2 + FONT_SIZE, FONT_SIZE, SCORE_COLOR);
        if (game.pause) {
            DrawText("PAUSE", WINDOW_WIDTH - FONT_SIZE * 4, FONT_SIZE / 2, FONT_SIZE, PAUSE_COLOR);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

