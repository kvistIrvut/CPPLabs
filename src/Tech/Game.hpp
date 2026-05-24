#ifndef GAME_H
#define GAME_H

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

    int m_selected_row = -1;
    int m_selected_col = -1;

    int m_score = 0;
    int m_moves = 0;
    int m_last_destroyed = 0;

    GameState m_state = GameState::Playing;

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void handleMouseButton(const int &button, const int &action, const int &mods);
    void handleKey(const int &key, const int &action, const int &mods);

    bool isInsideBoard(const double &mouse_x, const double &mouse_y) const;

    void clearSelection();
    void handleCellClick(const int &row, const int &col);

    void resetGame();
    void updateGameState();

    std::string getStateText() const;
    Color getStateColor() const;
    float getScoreProgress() const;
    int getScoreLeft() const;
    int getMovesLeft() const;

public:
    bool init();
    void run();
    void shutdown();
};

#endif //GAME_H
