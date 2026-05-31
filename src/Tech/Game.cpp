#include "Game.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <ctime>

#include "Containers/Config.hpp"

namespace {
float clampFloat(const float &value, const float &min_value, const float &max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

float vectorLength(const Vector2 &vector) {
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

Vector2 normalizeVector(const Vector2 &vector) {
    float length = vectorLength(vector);

    if (length <= 0.0001f) {
        return {0.0f, -1.0f};
    }

    return {vector.x / length, vector.y / length};
}

bool intersects(const Rect &first, const Rect &second) {
    return first.x < second.x + second.width &&
           first.x + first.width > second.x &&
           first.y < second.y + second.height &&
           first.y + first.height > second.y;
}

Rect getBallRect(const Ball &ball) {
    return {
            ball.position.x - ball.radius,
            ball.position.y - ball.radius,
            ball.radius * 2.0f,
            ball.radius * 2.0f
    };
}
}

Game::Game() {
    this->m_rng.seed(static_cast<unsigned int>(std::time(nullptr)) + 31U);
}

bool Game::init() {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ARKANOID", nullptr, nullptr);

    if (!this->m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(this->m_window);
    glfwSwapInterval(1);
    glfwSetWindowSizeLimits(this->m_window, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwSetWindowUserPointer(this->m_window, this);
    glfwSetKeyCallback(this->m_window, keyCallback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(this->m_window);
        glfwTerminate();
        this->m_window = nullptr;
        return false;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    resetGame();

    if (!this->m_renderer.init()) {
        shutdown();
        return false;
    }

    this->m_last_frame_time = static_cast<float>(glfwGetTime());
    return true;
}

void Game::run() {
    while (!glfwWindowShouldClose(this->m_window)) {
        float current_time = static_cast<float>(glfwGetTime());
        float delta_time = current_time - this->m_last_frame_time;
        this->m_last_frame_time = current_time;
        delta_time = clampFloat(delta_time, 0.0f, 0.033f);

        update(delta_time);

        glClearColor(0.07f, 0.08f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        this->m_renderer.renderGame(this->m_board,
                                    this->m_paddle,
                                    this->m_ball,
                                    this->m_score,
                                    this->m_lives,
                                    this->m_sticky_enabled,
                                    this->m_safety_floor_enabled,
                                    this->m_random_turn_enabled,
                                    getStateText(),
                                    getStateColor());

        glfwSwapBuffers(this->m_window);
        glfwPollEvents();
    }
}

void Game::shutdown() {
    this->m_renderer.shutdown();

    if (this->m_window) {
        glfwDestroyWindow(this->m_window);
        this->m_window = nullptr;
    }

    glfwTerminate();
}

void Game::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void) scancode;

    Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window));

    if (!game) {
        return;
    }

    game->handleKey(key, action, mods);
}

void Game::handleKey(const int &key, const int &action, const int &mods) {
    (void) mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(this->m_window, GLFW_TRUE);
        return;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        resetGame();
        return;
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && this->m_state == GameState::Playing) {
        launchBall();
        return;
    }

    bool is_pressed = action == GLFW_PRESS || action == GLFW_REPEAT;
    bool is_released = action == GLFW_RELEASE;

    if (key == GLFW_KEY_A) {
        if (is_pressed) {
            this->m_move_left = true;
        } else if (is_released) {
            this->m_move_left = false;
        }
    }

    if (key == GLFW_KEY_D) {
        if (is_pressed) {
            this->m_move_right = true;
        } else if (is_released) {
            this->m_move_right = false;
        }
    }
}

void Game::resetGame() {
    this->m_board.init();
    resetPaddle();
    resetBall();

    this->m_move_left = false;
    this->m_move_right = false;

    this->m_score = 0;
    this->m_lives = MAX_LIVES;
    this->m_ball_speed = BALL_START_SPEED;

    this->m_sticky_enabled = false;
    this->m_safety_floor_enabled = false;
    this->m_random_turn_enabled = false;
    this->m_random_turn_time_left = 0.0f;
    this->m_next_random_turn_time = 0.0f;

    this->m_state = GameState::Playing;
}

void Game::resetPaddle() {
    this->m_paddle.width = PADDLE_START_WIDTH;
    this->m_paddle.height = PADDLE_HEIGHT;
    this->m_paddle.x = FIELD_LEFT + FIELD_WIDTH * 0.5f - this->m_paddle.width * 0.5f;
    this->m_paddle.y = PADDLE_Y;
}

void Game::resetBall() {
    this->m_ball.radius = BALL_RADIUS;
    this->m_ball.position.x = this->m_paddle.x + this->m_paddle.width * 0.5f;
    this->m_ball.position.y = this->m_paddle.y - this->m_ball.radius - 1.0f;
    this->m_ball.velocity = {0.0f, 0.0f};
    this->m_ball.stuck_to_paddle = true;
}

void Game::launchBall() {
    if (!this->m_ball.stuck_to_paddle) {
        return;
    }

    float horizontal_direction = randomFloat(-0.35f, 0.35f);
    Vector2 direction = normalizeVector({horizontal_direction, -1.0f});

    this->m_ball.velocity.x = direction.x * this->m_ball_speed;
    this->m_ball.velocity.y = direction.y * this->m_ball_speed;
    this->m_ball.stuck_to_paddle = false;
}

void Game::update(const float &delta_time) {
    if (this->m_state != GameState::Playing) {
        return;
    }

    updatePaddle(delta_time);
    updateRandomTurn(delta_time);
    updateBall(delta_time);
    updateBonuses(delta_time);

    if (!this->m_board.hasRemainingDestructibleBlocks()) {
        this->m_state = GameState::Won;
        this->m_ball.velocity = {0.0f, 0.0f};
        this->m_ball.stuck_to_paddle = true;
    }
}

void Game::updatePaddle(const float &delta_time) {
    float direction = 0.0f;

    if (this->m_move_left) {
        direction -= 1.0f;
    }
    if (this->m_move_right) {
        direction += 1.0f;
    }

    this->m_paddle.x += direction * PADDLE_SPEED * delta_time;
    this->m_paddle.x = clampFloat(this->m_paddle.x, FIELD_LEFT, FIELD_RIGHT - this->m_paddle.width);

    if (this->m_ball.stuck_to_paddle) {
        this->m_ball.position.x = this->m_paddle.x + this->m_paddle.width * 0.5f;
        this->m_ball.position.y = this->m_paddle.y - this->m_ball.radius - 1.0f;
    }
}

void Game::updateBall(const float &delta_time) {
    if (this->m_ball.stuck_to_paddle) {
        return;
    }

    this->m_ball.position.x += this->m_ball.velocity.x * delta_time;
    this->m_ball.position.y += this->m_ball.velocity.y * delta_time;

    handleWallCollision();
    handlePaddleCollision();

    BrickHitResult hit_result = this->m_board.handleBallCollision(this->m_ball);
    if (hit_result.hit) {
        this->m_score += hit_result.score;

        if (hit_result.speed_up_ball) {
            multiplyBallSpeed(SPEED_BLOCK_FACTOR);
        }
    }

    handleBottomCollision();
}

void Game::updateBonuses(const float &delta_time) {
    this->m_board.updateBonuses(delta_time);

    BonusType bonus_type = BonusType::PaddleGrow;

    while (this->m_board.collectBonus(getPaddleRect(), bonus_type)) {
        applyBonus(bonus_type);
    }
}

void Game::updateRandomTurn(const float &delta_time) {
    if (!this->m_random_turn_enabled) {
        return;
    }

    this->m_random_turn_time_left -= delta_time;

    if (this->m_random_turn_time_left <= 0.0f) {
        this->m_random_turn_enabled = false;
        this->m_next_random_turn_time = 0.0f;
        return;
    }

    if (this->m_ball.stuck_to_paddle) {
        return;
    }

    this->m_next_random_turn_time -= delta_time;

    if (this->m_next_random_turn_time <= 0.0f) {
        randomizeBallDirection();
        this->m_next_random_turn_time = getRandomTurnDelay();
    }
}

void Game::handleWallCollision() {
    if (this->m_ball.position.x - this->m_ball.radius <= FIELD_LEFT) {
        this->m_ball.position.x = FIELD_LEFT + this->m_ball.radius;
        this->m_ball.velocity.x = std::fabs(this->m_ball.velocity.x);
    }

    if (this->m_ball.position.x + this->m_ball.radius >= FIELD_RIGHT) {
        this->m_ball.position.x = FIELD_RIGHT - this->m_ball.radius;
        this->m_ball.velocity.x = -std::fabs(this->m_ball.velocity.x);
    }

    if (this->m_ball.position.y - this->m_ball.radius <= FIELD_TOP) {
        this->m_ball.position.y = FIELD_TOP + this->m_ball.radius;
        this->m_ball.velocity.y = std::fabs(this->m_ball.velocity.y);
    }
}

void Game::handlePaddleCollision() {
    if (this->m_ball.velocity.y <= 0.0f) {
        return;
    }

    if (!intersects(getBallRect(this->m_ball), getPaddleRect())) {
        return;
    }

    this->m_ball.position.y = this->m_paddle.y - this->m_ball.radius - 1.0f;

    if (this->m_sticky_enabled) {
        this->m_ball.velocity = {0.0f, 0.0f};
        this->m_ball.stuck_to_paddle = true;
        this->m_sticky_enabled = false;
        return;
    }

    float paddle_center = this->m_paddle.x + this->m_paddle.width * 0.5f;
    float relative_hit = (this->m_ball.position.x - paddle_center) / (this->m_paddle.width * 0.5f);
    relative_hit = clampFloat(relative_hit, -1.0f, 1.0f);

    this->m_ball.velocity.x = relative_hit * this->m_ball_speed * 0.85f;
    float y_square = this->m_ball_speed * this->m_ball_speed - this->m_ball.velocity.x * this->m_ball.velocity.x;
    this->m_ball.velocity.y = -std::sqrt(std::max(0.0f, y_square));
}

void Game::handleBottomCollision() {
    if (this->m_ball.position.y + this->m_ball.radius >= FIELD_BOTTOM && this->m_safety_floor_enabled) {
        this->m_ball.position.y = FIELD_BOTTOM - this->m_ball.radius - 1.0f;
        this->m_ball.velocity.y = -std::fabs(this->m_ball.velocity.y);
        this->m_safety_floor_enabled = false;
        return;
    }

    if (this->m_ball.position.y - this->m_ball.radius > FIELD_BOTTOM) {
        handleBallLost();
    }
}

void Game::handleBallLost() {
    this->m_lives = std::max(0, this->m_lives - 1);
    this->m_score -= MISS_SCORE_PENALTY;
    this->m_sticky_enabled = false;
    this->m_safety_floor_enabled = false;
    this->m_random_turn_enabled = false;

    if (this->m_lives <= 0) {
        this->m_state = GameState::Lost;
        this->m_ball.velocity = {0.0f, 0.0f};
        this->m_ball.stuck_to_paddle = true;
        return;
    }

    resetBall();
}

void Game::applyBonus(const BonusType &bonus_type) {
    float paddle_center = this->m_paddle.x + this->m_paddle.width * 0.5f;

    switch (bonus_type) {
        case BonusType::PaddleGrow:
            this->m_paddle.width = std::min(PADDLE_MAX_WIDTH, this->m_paddle.width * 1.25f);
            this->m_paddle.x = clampFloat(paddle_center - this->m_paddle.width * 0.5f, FIELD_LEFT, FIELD_RIGHT - this->m_paddle.width);
            break;
        case BonusType::PaddleShrink:
            this->m_paddle.width = std::max(PADDLE_MIN_WIDTH, this->m_paddle.width * 0.80f);
            this->m_paddle.x = clampFloat(paddle_center - this->m_paddle.width * 0.5f, FIELD_LEFT, FIELD_RIGHT - this->m_paddle.width);
            break;
        case BonusType::BallSpeedUp:
            multiplyBallSpeed(BALL_SPEED_BONUS_FACTOR);
            break;
        case BonusType::BallSpeedDown:
            multiplyBallSpeed(BALL_SPEED_PENALTY_FACTOR);
            break;
        case BonusType::StickyPaddle:
            this->m_sticky_enabled = true;
            break;
        case BonusType::SafetyFloor:
            this->m_safety_floor_enabled = true;
            break;
        case BonusType::RandomDirection:
            this->m_random_turn_enabled = true;
            this->m_random_turn_time_left = RANDOM_TURN_TOTAL_TIME;
            this->m_next_random_turn_time = getRandomTurnDelay();
            if (!this->m_ball.stuck_to_paddle) {
                randomizeBallDirection();
            }
            break;
        default:
            break;
    }
}

void Game::setBallSpeed(const float &new_speed) {
    this->m_ball_speed = clampFloat(new_speed, BALL_MIN_SPEED, BALL_MAX_SPEED);

    if (this->m_ball.stuck_to_paddle) {
        return;
    }

    Vector2 direction = normalizeVector(this->m_ball.velocity);
    this->m_ball.velocity.x = direction.x * this->m_ball_speed;
    this->m_ball.velocity.y = direction.y * this->m_ball_speed;
}

void Game::multiplyBallSpeed(const float &factor) {
    setBallSpeed(this->m_ball_speed * factor);
}

void Game::randomizeBallDirection() {
    if (this->m_ball.stuck_to_paddle) {
        return;
    }

    float min_degrees = this->m_ball.velocity.y < 0.0f ? -160.0f : 20.0f;
    float max_degrees = this->m_ball.velocity.y < 0.0f ? -20.0f : 160.0f;
    float angle = randomFloat(min_degrees, max_degrees) * 3.14159265359f / 180.0f;

    this->m_ball.velocity.x = std::cos(angle) * this->m_ball_speed;
    this->m_ball.velocity.y = std::sin(angle) * this->m_ball_speed;
}

Rect Game::getPaddleRect() const {
    return {this->m_paddle.x, this->m_paddle.y, this->m_paddle.width, this->m_paddle.height};
}

float Game::randomFloat(const float &min_value, const float &max_value) {
    std::uniform_real_distribution<float> dist(min_value, max_value);
    return dist(this->m_rng);
}

float Game::getRandomTurnDelay() {
    return randomFloat(RANDOM_TURN_MIN_DELAY, RANDOM_TURN_MAX_DELAY);
}

std::string Game::getStateText() const {
    switch (this->m_state) {
        case GameState::Playing:
            if (this->m_ball.stuck_to_paddle) {
                return "SPACE TO LAUNCH";
            }
            return "PLAYING";
        case GameState::Won:
            return "WON PRESS R";
        case GameState::Lost:
            return "LOST PRESS R";
        default:
            return "UNKNOWN";
    }
}

Color Game::getStateColor() const {
    switch (this->m_state) {
        case GameState::Playing:
            return {0.82f, 0.84f, 0.90f};
        case GameState::Won:
            return {0.20f, 0.85f, 0.35f};
        case GameState::Lost:
            return {0.95f, 0.20f, 0.25f};
        default:
            return {1.0f, 1.0f, 1.0f};
    }
}
