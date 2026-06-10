#ifndef CONFIG_H
#define CONFIG_H

constexpr int WINDOW_WIDTH = 900;
constexpr int WINDOW_HEIGHT = 700;

constexpr int BOARD_ROWS = 8;
constexpr int BOARD_COLS = 8;

constexpr int COLOR_COUNT = 6;
constexpr int EMPTY_CELL = -1;

constexpr float CELL_SIZE = 70.0f;

constexpr float BOARD_PIXEL_WIDTH = BOARD_COLS * CELL_SIZE;
constexpr float BOARD_PIXEL_HEIGHT = BOARD_ROWS * CELL_SIZE;

constexpr float BOARD_OFFSET_X = (WINDOW_WIDTH - BOARD_PIXEL_WIDTH) / 2.0f;
constexpr float BOARD_OFFSET_Y = (WINDOW_HEIGHT - BOARD_PIXEL_HEIGHT) / 2.0f;

constexpr int BONUS_RADIUS = 3;
constexpr int BONUS_CHANCE_PERCENT = 12;

constexpr int MAX_MOVES = 30;
constexpr int TARGET_SCORE = 1000;
constexpr int SCORE_PER_DESTROYED_CELL = 10;

#endif //CONFIG_H
