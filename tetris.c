#include "raylib.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 800
#define FPS 60

#define BLOCK_LENGTH 50
#define PLAYABLE_WIDTH (WINDOW_WIDTH / BLOCK_LENGTH)
#define PLAYABLE_HEIGHT (WINDOW_HEIGHT / BLOCK_LENGTH)

#define WALL_THICKNESS 2
#define TABLE_WIDTH (PLAYABLE_WIDTH + WALL_THICKNESS * 2)
#define TABLE_HEIGHT (PLAYABLE_HEIGHT + WALL_THICKNESS * 2)

#define SHAPE_FRAME_LENGTH 4

#define UINT64_HIGH_1_BIT (0x1ULL << 63)
#define UINT64_HIGH_4_BIT (0xFULL << 60)

#define UINT16_HIGH_1_BIT (0x1U << 15)
#define UINT16_HIGH_4_BIT (0xFU << 12)

#define CENTRE_X (TABLE_WIDTH / 2 - SHAPE_FRAME_LENGTH / 2)
#define START_X CENTRE_X
#define START_Y WALL_THICKNESS

#define ROTATE_ID(id, r) (((id) & ~3) | (((id) + (r) + 4) & 3))

#define EMPTY_LINE_MASK ((UINT64_MAX >> (PLAYABLE_WIDTH + WALL_THICKNESS)) | (UINT64_MAX << (64 - WALL_THICKNESS)))
#define FULL_LINE_MASK (~EMPTY_LINE_MASK)

// NULL, I, O, T, S, Z, J, L
const uint16_t shapes[32] = {
    0, 0, 0, 0,
    0b0000111100000000, 0b0010001000100010, 0b0000000011110000, 0b0100010001000100,
    0b1100110000000000, 0b1100110000000000, 0b1100110000000000, 0b1100110000000000,
    0b1110010000000000, 0b0010011000100000, 0b0000010011100000, 0b1000110010000000,
    0b0110110000000000, 0b0100011000100000, 0b0000011011000000, 0b1000110001000000,
    0b1100011000000000, 0b0010011001000000, 0b0000110001100000, 0b0100110010000000,
    0b1000111000000000, 0b0110010001000000, 0b0000111000100000, 0b0100010011000000,
    0b1110100000000000, 0b0110001000100000, 0b0000001011100000, 0b1000100011000000,
};

void InitTable(uint64_t table[]) {
    for (int y = 0; y < WALL_THICKNESS; ++y) {
        table[y] = UINT64_MAX;
        table[TABLE_HEIGHT - 1 - y] = UINT64_MAX;
    }
    for (int y = WALL_THICKNESS; y < TABLE_HEIGHT - WALL_THICKNESS; ++y) {
        table[y] = (UINT64_MAX >> (PLAYABLE_WIDTH + WALL_THICKNESS)) | (UINT64_MAX << (64 - WALL_THICKNESS));
    }
}

void PrintTable(const uint64_t table[]) {
    for (int y = 0; y < TABLE_HEIGHT; ++y) {
        printf("[y = %2d]", y);
        for (int x = 0; x < 64; ++x) {
            if (!(x % 4)) putchar(' ');
            uint64_t bit = table[y] & (UINT64_HIGH_1_BIT >> x);
            putchar(bit ? '#' : '.');
        }
        putchar('\n');
    }
    putchar('\n');
}

void DrawTable(const uint64_t table[]) {
    for (int y = 0; y < PLAYABLE_HEIGHT; ++y) {
        for (int x = 0; x < PLAYABLE_WIDTH; ++x) {
            uint64_t bit = table[y + WALL_THICKNESS] & (UINT64_HIGH_1_BIT >> (x + WALL_THICKNESS));
            Color color = bit ? WHITE : BLANK;
            DrawRectangle(x * BLOCK_LENGTH, y * BLOCK_LENGTH, BLOCK_LENGTH, BLOCK_LENGTH, color);
        }
    }
}

typedef struct {
    uint64_t* table;
    int shapeId, curX, curY, score;
    float fallTime;
} Scene;

void DrawShape(const Scene* scene) {
    uint16_t shape = shapes[scene->shapeId];
    int curX = scene->curX;
    int curY = scene->curY;
    for (int row = 0; row < SHAPE_FRAME_LENGTH; ++row) {
        for (int col = 0; col < SHAPE_FRAME_LENGTH; ++col) {
            uint16_t lineOfShapeFrame = (shape << (row * SHAPE_FRAME_LENGTH)) & UINT16_HIGH_4_BIT;
            uint16_t bit = lineOfShapeFrame & (UINT16_HIGH_1_BIT >> col);
//            printf("%d\n", bit);
            Color color = bit ? WHITE : BLANK;
            DrawRectangle((curX + col - WALL_THICKNESS) * BLOCK_LENGTH, (curY + row - WALL_THICKNESS) * BLOCK_LENGTH, BLOCK_LENGTH, BLOCK_LENGTH, color);
        }
    }
//    printf("-----------------------\n");
}

bool CanMove(const Scene* scene, int offsetX, int offsetY, int rotate) {
    int shapeId = scene->shapeId;
    if (rotate) shapeId = ROTATE_ID(shapeId, rotate);
    uint64_t shape = (uint64_t)shapes[shapeId] << 48;
    uint64_t* table = scene->table;
    int curX = scene->curX;
    int curY = scene->curY;
    if ((curX + offsetX) < 0 || (curX + offsetX) > TABLE_WIDTH) return false;
    for (int row = 0; row < SHAPE_FRAME_LENGTH; ++row) {
        uint64_t lineOfShapeFrame = ((shape << (row * SHAPE_FRAME_LENGTH)) & UINT64_HIGH_4_BIT) >> (curX + offsetX);
        uint64_t lineOfTable = table[curY + row + offsetY];
        uint64_t isOverlap = lineOfShapeFrame & lineOfTable;
        if (isOverlap) return false;
    }
    return true;
}

bool Update(Scene* scene, int offsetX, int offsetY, int rotate) {
    if (CanMove(scene, offsetX, offsetY, rotate)) {
        scene->curX += offsetX;
        scene->curY += offsetY;
        scene->shapeId = ROTATE_ID(scene->shapeId, rotate);
        return true;
    }
    return false;
}

void HandleInput(Scene* scene) {
    if (IsKeyPressed(KEY_H)) Update(scene, -1, 0, 0);
    if (IsKeyPressed(KEY_L)) Update(scene, 1, 0, 0);
    if (IsKeyPressed(KEY_J)) Update(scene, 0, 0, -1);
    if (IsKeyPressed(KEY_K)) Update(scene, 0, 0, 1);
    if (IsKeyDown(KEY_SPACE)) scene->fallTime = 0.1f;
    else scene->fallTime = 0.6f;
}

void CommitShape(Scene* scene) {
    uint64_t shape = (uint64_t)shapes[scene->shapeId] << 48;
    uint64_t* table = scene->table;
    int curX = scene->curX;
    int curY = scene->curY;
    for (int row = 0; row < SHAPE_FRAME_LENGTH; ++row) {
        uint64_t lineOfShapeFrame = ((shape << (row * SHAPE_FRAME_LENGTH)) & UINT64_HIGH_4_BIT) >> curX;
        uint64_t* lineOfTable = &table[curY + row];
        *lineOfTable |= lineOfShapeFrame;
    }
}

void NewStart(Scene* scene) {
    scene->shapeId = GetRandomValue(0, 1000) % 28 + 4;
    scene->curX = START_X;
    scene->curY = START_Y;
}

// TODO: Improve the CheckLines algorithm
void CheckLines(Scene* scene) {
    uint64_t* table = scene->table;
    for (int y = TABLE_HEIGHT - WALL_THICKNESS - 1; y >= WALL_THICKNESS; --y) {
        if ((table[y] & FULL_LINE_MASK) == FULL_LINE_MASK) {
            scene->score += 10;
            for (int t = y; t >= WALL_THICKNESS + 1; --t) {
                table[t] = table[t - 1];
            }
            y = TABLE_HEIGHT - WALL_THICKNESS - 1;
        }
    }
}

void NaturalFall(Scene* scene) {
    if (!Update(scene, 0, 1, 0)) {
        CommitShape(scene);
        CheckLines(scene);
        NewStart(scene);
    }
}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris");
    SetTargetFPS(120);
    SetExitKey(KEY_Q);

    float timer = 0.f;
    uint64_t table[TABLE_HEIGHT] = {0};
    InitTable(table);
    PrintTable(table);

    Scene scene = { table, GetRandomValue(0, 1000) % 28 + 4, START_X, START_Y, 0, 0.6f };

    while (!WindowShouldClose()) {
        HandleInput(&scene);

        BeginDrawing();
        ClearBackground(BLACK);

        timer += GetFrameTime(); 
        if (timer >= scene.fallTime) {
            NaturalFall(&scene);
            timer -= scene.fallTime;
        }

        DrawTable(table);
        DrawShape(&scene);
        DrawText(TextFormat("%d", scene.score), WINDOW_WIDTH - 80, 40, 40, WHITE);

        // DrawFPS(5, 5);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

