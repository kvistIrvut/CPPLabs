#ifndef GAME_H
#define GAME_H

#include <random>
#include <string>

#include "Tech/Board.hpp"
#include "Tech/Renderer.hpp"
#include "Containers/Types.hpp"

struct GLFWwindow;

class Game {
private:
    GLFWwindow *m_window = nullptr;

    Board m_board;
    Renderer m_renderer;

    Paddle m_paddle;
    Ball m_ball;

    bool m_move_left = false;
    bool m_move_right = false;

    int m_score = 0;
    int m_lives = START_LIVES;

    float m_ball_speed = BALL_START_SPEED;
    float m_last_frame_time = 0.0f;

    bool m_sticky_enabled = false;
    bool m_safety_floor_enabled = false;
    bool m_random_turn_enabled = false;
    float m_random_turn_time_left = 0.0f;
    float m_next_random_turn_time = 0.0f;

    GameState m_state = GameState::Playing;
    std::mt19937 m_rng;

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void handleKey(const int &key, const int &action, const int &mods);

    void resetGame();
    void resetPaddle();
    void resetBall();
    void launchBall();

    void update(const float &delta_time);
    void updatePaddle(const float &delta_time);
    void updateBall(const float &delta_time);
    void updateBonuses(const float &delta_time);
    void updateRandomTurn(const float &delta_time);

    void handleWallCollision();
    void handlePaddleCollision();
    void handleBottomCollision();
    void handleBallLost();

    void applyBonus(const BonusType &bonus_type);
    void setBallSpeed(const float &new_speed);
    void multiplyBallSpeed(const float &factor);
    void randomizeBallDirection();

    Rect getPaddleRect() const;
    float randomFloat(const float &min_value, const float &max_value);
    float getRandomTurnDelay();

    std::string getStateText() const;
    Color getStateColor() const;

public:
    Game();

    bool init();
    void run();
    void shutdown();
};

#endif //GAME_H
