#ifndef CONFIG_H
#define CONFIG_H

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 700;

const float FIELD_LEFT = 40.0f;
const float FIELD_TOP = 70.0f;
const float FIELD_WIDTH = 820.0f;
const float FIELD_HEIGHT = 580.0f;
const float FIELD_RIGHT = FIELD_LEFT + FIELD_WIDTH;
const float FIELD_BOTTOM = FIELD_TOP + FIELD_HEIGHT;

const int BRICK_ROWS = 6;
const int BRICK_COLS = 10;
const int UNBREAKABLE_BRICK_COUNT = 8;
const int SPEED_UP_BRICK_COUNT = 8;

const float BRICK_GAP = 6.0f;
const float BRICK_WIDTH = (FIELD_WIDTH - BRICK_GAP * static_cast<float>(BRICK_COLS + 1)) / static_cast<float>(BRICK_COLS);
const float BRICK_HEIGHT = 30.0f;
const float BRICK_START_X = FIELD_LEFT + BRICK_GAP;
const float BRICK_START_Y = FIELD_TOP + 36.0f;

const float PADDLE_START_WIDTH = 125.0f;
const float PADDLE_MIN_WIDTH = 70.0f;
const float PADDLE_MAX_WIDTH = 190.0f;
const float PADDLE_HEIGHT = 16.0f;
const float PADDLE_Y = FIELD_BOTTOM - 38.0f;
const float PADDLE_SPEED = 520.0f;

const float BALL_RADIUS = 8.0f;
const float BALL_START_SPEED = 330.0f;
const float BALL_MIN_SPEED = 230.0f;
const float BALL_MAX_SPEED = 620.0f;
const float BALL_SPEED_BONUS_FACTOR = 1.16f;
const float BALL_SPEED_PENALTY_FACTOR = 0.84f;
const float SPEED_BLOCK_FACTOR = 1.08f;

const float BONUS_WIDTH = 26.0f;
const float BONUS_HEIGHT = 18.0f;
const float BONUS_FALL_SPEED = 180.0f;
const int BONUS_CHANCE_PERCENT = 45;

const int MAX_LIVES = 3;
const int START_LIVES = MAX_LIVES;
const int SCORE_PER_BRICK_HIT = 1;
const int MISS_SCORE_PENALTY = 5;

const float RANDOM_TURN_TOTAL_TIME = 8.0f;
const float RANDOM_TURN_MIN_DELAY = 0.75f;
const float RANDOM_TURN_MAX_DELAY = 1.80f;

#endif //CONFIG_H
