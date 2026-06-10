#ifndef TYPES_H
#define TYPES_H

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct Cell {
    int color_index = 0;
    bool selected = false;
};

struct Vertex {
    float x = 0.0f;
    float y = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct DestroyedCell {
    int row = 0;
    int col = 0;
    int original_color = 0;
};

enum class BonusType {
    Repaint,
    Bomb
};

enum class GameState {
    Playing,
    Won,
    Lost
};

#endif //TYPES_H
