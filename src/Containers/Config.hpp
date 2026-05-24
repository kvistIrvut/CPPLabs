#ifndef CONFIG_H
#define CONFIG_H

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 700;

const int BOARD_ROWS = 8;
const int BOARD_COLS = 8;

const int COLOR_COUNT = 6;
const int EMPTY_CELL = -1;

const float CELL_SIZE = 70.0f;

const float BOARD_PIXEL_WIDTH = BOARD_COLS * CELL_SIZE;
const float BOARD_PIXEL_HEIGHT = BOARD_ROWS * CELL_SIZE;

const float BOARD_OFFSET_X = (WINDOW_WIDTH - BOARD_PIXEL_WIDTH) / 2.0f;
const float BOARD_OFFSET_Y = (WINDOW_HEIGHT - BOARD_PIXEL_HEIGHT) / 2.0f;

const int BONUS_RADIUS = 3;
const int BONUS_CHANCE_PERCENT = 12;

const int MAX_MOVES = 30;
const int TARGET_SCORE = 1000;
const int SCORE_PER_DESTROYED_CELL = 10;

#endif //CONFIG_H
