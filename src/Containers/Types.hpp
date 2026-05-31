#ifndef TYPES_H
#define TYPES_H

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct Vertex {
    float x = 0.0f;
    float y = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct Ball {
    Vector2 position;
    Vector2 velocity;
    float radius = 0.0f;
    bool stuck_to_paddle = true;
};

struct Paddle {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

enum class BrickType {
    Empty,
    Normal,
    Unbreakable,
    SpeedUp
};

struct Brick {
    BrickType type = BrickType::Empty;
    int health = 0;
    bool has_bonus = false;
};

enum class BonusType {
    PaddleGrow,
    PaddleShrink,
    BallSpeedUp,
    BallSpeedDown,
    StickyPaddle,
    SafetyFloor,
    RandomDirection
};

struct Bonus {
    BonusType type = BonusType::PaddleGrow;
    Rect rect;
    bool active = false;
};

struct BrickHitResult {
    bool hit = false;
    int score = 0;
    bool speed_up_ball = false;
};

enum class GameState {
    Playing,
    Won,
    Lost
};

#endif //TYPES_H
