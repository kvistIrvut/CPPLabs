#include "Game.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Containers/Config.hpp"

namespace {
float clamp01(const float &value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}
}

bool Game::init() {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "GEMS", nullptr, nullptr);

    if (!this->m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(this->m_window);
    glfwSetWindowSizeLimits(this->m_window, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwSetWindowUserPointer(this->m_window, this);
    glfwSetMouseButtonCallback(this->m_window, mouseButtonCallback);
    glfwSetKeyCallback(this->m_window, keyCallback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(this->m_window);
        glfwTerminate();
        this->m_window = nullptr;
        return false;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    this->m_board.init();
    this->m_board.processCascades(false);

    if (!this->m_renderer.init()) {
        shutdown();
        return false;
    }

    return true;
}

void Game::run() {
    while (!glfwWindowShouldClose(this->m_window)) {
        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        this->m_renderer.renderGame(this->m_board,
                                    getStateColor(),
                                    getScoreProgress(),
                                    this->m_score,
                                    getScoreLeft(),
                                    getMovesLeft(),
                                    getStateText());

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

void Game::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window));

    if (!game) {
        return;
    }

    game->handleMouseButton(button, action, mods);
}

void Game::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void) scancode;

    Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window));

    if (!game) {
        return;
    }

    game->handleKey(key, action, mods);
}

void Game::handleMouseButton(const int &button, const int &action, const int &mods) {
    (void) mods;

    if (this->m_state != GameState::Playing) {
        return;
    }
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
        return;
    }

    double mouse_x = 0.0;
    double mouse_y = 0.0;

    glfwGetCursorPos(this->m_window, &mouse_x, &mouse_y);

    if (!isInsideBoard(mouse_x, mouse_y)) {
        clearSelection();
        return;
    }

    int col = static_cast<int>((mouse_x - BOARD_OFFSET_X) / CELL_SIZE);
    int row = static_cast<int>((mouse_y - BOARD_OFFSET_Y) / CELL_SIZE);

    if (Board::isValidCell(row, col)) {
        handleCellClick(row, col);
    }
}

void Game::handleKey(const int &key, const int &action, const int &mods) {
    (void) mods;

    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(this->m_window, GLFW_TRUE);
        return;
    }
    if (key == GLFW_KEY_R) {
        resetGame();
        return;
    }
}

bool Game::isInsideBoard(const double &mouse_x, const double &mouse_y) const {
    return mouse_x >= BOARD_OFFSET_X &&
           mouse_x < BOARD_OFFSET_X + BOARD_PIXEL_WIDTH &&
           mouse_y >= BOARD_OFFSET_Y &&
           mouse_y < BOARD_OFFSET_Y + BOARD_PIXEL_HEIGHT;
}

void Game::clearSelection() {
    if (this->m_selected_row != -1 && this->m_selected_col != -1) {
        this->m_board.setSelected(this->m_selected_row, this->m_selected_col, false);
    }

    this->m_selected_row = -1;
    this->m_selected_col = -1;
}

void Game::handleCellClick(const int &row, const int &col) {
    if (this->m_state != GameState::Playing) {
        return;
    }

    if (this->m_selected_row == -1 || this->m_selected_col == -1) {
        this->m_selected_row = row;
        this->m_selected_col = col;
        this->m_board.setSelected(row, col, true);
        return;
    }

    if (this->m_selected_row == row && this->m_selected_col == col) {
        clearSelection();
        return;
    }

    if (Board::areAdjacent(this->m_selected_row, this->m_selected_col, row, col)) {
        int first_row = this->m_selected_row;
        int first_col = this->m_selected_col;

        clearSelection();
        this->m_board.swapCells(first_row, first_col, row, col);

        int destroyed = this->m_board.processCascades(true);
        this->m_last_destroyed = destroyed;

        if (destroyed == 0) {
            this->m_board.swapCells(first_row, first_col, row, col);
            return;
        }

        this->m_moves++;

        int gained_score = destroyed * SCORE_PER_DESTROYED_CELL;
        this->m_score += gained_score;

        updateGameState();
        return;
    }

    clearSelection();

    this->m_selected_row = row;
    this->m_selected_col = col;
    this->m_board.setSelected(row, col, true);
}

void Game::updateGameState() {
    if (this->m_score >= TARGET_SCORE) {
        this->m_state = GameState::Won;
        clearSelection();
        return;
    }

    if (this->m_moves >= MAX_MOVES) {
        this->m_state = GameState::Lost;
        clearSelection();
        return;
    }

    this->m_state = GameState::Playing;
}

std::string Game::getStateText() const {
    switch (this->m_state) {
        case GameState::Playing:
            return "PLAYING";
        case GameState::Won:
            return "YOU WON";
        case GameState::Lost:
            return "YOU LOST";
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

float Game::getScoreProgress() const {
    if (TARGET_SCORE <= 0) {
        return 1.0f;
    }

    return clamp01(static_cast<float>(this->m_score) / static_cast<float>(TARGET_SCORE));
}

int Game::getScoreLeft() const {
    int score_left = TARGET_SCORE - this->m_score;

    if (score_left < 0) {
        return 0;
    }

    return score_left;
}

int Game::getMovesLeft() const {
    int moves_left = MAX_MOVES - this->m_moves;

    if (moves_left < 0) {
        return 0;
    }

    return moves_left;
}

void Game::resetGame() {
    clearSelection();

    this->m_board.init();
    this->m_board.processCascades(false);

    this->m_score = 0;
    this->m_moves = 0;
    this->m_last_destroyed = 0;
    this->m_state = GameState::Playing;
}
