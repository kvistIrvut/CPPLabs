#include "Board.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <vector>

Board::Board() {
    this->m_rng.seed(static_cast<unsigned int>(std::time(nullptr)));
}

void Board::init() {
    this->m_bonuses.clear();

    std::vector<BrickType> brick_types;
    brick_types.reserve(BRICK_ROWS * BRICK_COLS);

    int unbreakable_count = std::min(UNBREAKABLE_BRICK_COUNT, BRICK_ROWS * BRICK_COLS - 1);
    int speed_up_count = std::min(SPEED_UP_BRICK_COUNT, BRICK_ROWS * BRICK_COLS - unbreakable_count);
    int normal_count = BRICK_ROWS * BRICK_COLS - unbreakable_count - speed_up_count;

    for (int i = 0; i < unbreakable_count; i++) {
        brick_types.push_back(BrickType::Unbreakable);
    }
    for (int i = 0; i < speed_up_count; i++) {
        brick_types.push_back(BrickType::SpeedUp);
    }
    for (int i = 0; i < normal_count; i++) {
        brick_types.push_back(BrickType::Normal);
    }

    std::shuffle(brick_types.begin(), brick_types.end(), this->m_rng);

    int brick_index = 0;
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            this->m_bricks[row][col] = createBrick(brick_types[brick_index]);
            brick_index++;
        }
    }
}

Brick Board::createBrick(const BrickType &type) {
    Brick brick;
    brick.type = type;

    if (type == BrickType::Unbreakable) {
        brick.health = -1;
        brick.has_bonus = false;
        return brick;
    }

    if (type == BrickType::SpeedUp) {
        brick.health = randomInt(1, 2);
    } else {
        brick.health = randomInt(1, 3);
    }

    brick.has_bonus = randomChance(BONUS_CHANCE_PERCENT);
    return brick;
}

int Board::randomInt(const int &min_value, const int &max_value) {
    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(this->m_rng);
}

float Board::randomFloat(const float &min_value, const float &max_value) {
    std::uniform_real_distribution<float> dist(min_value, max_value);
    return dist(this->m_rng);
}

bool Board::randomChance(const int &percent) {
    return randomInt(1, 100) <= percent;
}

BonusType Board::randomBonusType() {
    switch (randomInt(0, 6)) {
        case 0:
            return BonusType::PaddleGrow;
        case 1:
            return BonusType::PaddleShrink;
        case 2:
            return BonusType::BallSpeedUp;
        case 3:
            return BonusType::BallSpeedDown;
        case 4:
            return BonusType::StickyPaddle;
        case 5:
            return BonusType::SafetyFloor;
        default:
            return BonusType::RandomDirection;
    }
}

bool Board::isValidBrick(const int &row, const int &col) {
    return row >= 0 && row < BRICK_ROWS && col >= 0 && col < BRICK_COLS;
}

Rect Board::getBrickRect(const int &row, const int &col) {
    return {
            BRICK_START_X + static_cast<float>(col) * (BRICK_WIDTH + BRICK_GAP),
            BRICK_START_Y + static_cast<float>(row) * (BRICK_HEIGHT + BRICK_GAP),
            BRICK_WIDTH,
            BRICK_HEIGHT
    };
}

Rect Board::getBrickScreenRect(const int &row, const int &col) {
    return getBrickRect(row, col);
}

Rect Board::getBallRect(const Ball &ball) {
    return {
            ball.position.x - ball.radius,
            ball.position.y - ball.radius,
            ball.radius * 2.0f,
            ball.radius * 2.0f
    };
}

bool Board::intersects(const Rect &first, const Rect &second) {
    return first.x < second.x + second.width &&
           first.x + first.width > second.x &&
           first.y < second.y + second.height &&
           first.y + first.height > second.y;
}

float Board::getOverlapX(const Rect &first, const Rect &second) {
    float first_center = first.x + first.width * 0.5f;
    float second_center = second.x + second.width * 0.5f;
    float distance = std::fabs(first_center - second_center);

    return first.width * 0.5f + second.width * 0.5f - distance;
}

float Board::getOverlapY(const Rect &first, const Rect &second) {
    float first_center = first.y + first.height * 0.5f;
    float second_center = second.y + second.height * 0.5f;
    float distance = std::fabs(first_center - second_center);

    return first.height * 0.5f + second.height * 0.5f - distance;
}

void Board::reflectBallFromRect(Ball &ball, const Rect &rect) const {
    Rect ball_rect = getBallRect(ball);
    float overlap_x = getOverlapX(ball_rect, rect);
    float overlap_y = getOverlapY(ball_rect, rect);

    if (overlap_x < overlap_y) {
        if (ball.position.x < rect.x + rect.width * 0.5f) {
            ball.position.x = rect.x - ball.radius;
            ball.velocity.x = -std::fabs(ball.velocity.x);
        } else {
            ball.position.x = rect.x + rect.width + ball.radius;
            ball.velocity.x = std::fabs(ball.velocity.x);
        }
        return;
    }

    if (ball.position.y < rect.y + rect.height * 0.5f) {
        ball.position.y = rect.y - ball.radius;
        ball.velocity.y = -std::fabs(ball.velocity.y);
    } else {
        ball.position.y = rect.y + rect.height + ball.radius;
        ball.velocity.y = std::fabs(ball.velocity.y);
    }
}

void Board::spawnBonus(const int &row, const int &col) {
    Rect brick_rect = getBrickRect(row, col);

    Bonus bonus;
    bonus.type = randomBonusType();
    bonus.rect.width = BONUS_WIDTH;
    bonus.rect.height = BONUS_HEIGHT;
    bonus.rect.x = brick_rect.x + brick_rect.width * 0.5f - BONUS_WIDTH * 0.5f;
    bonus.rect.y = brick_rect.y + brick_rect.height * 0.5f - BONUS_HEIGHT * 0.5f;
    bonus.active = true;

    this->m_bonuses.push_back(bonus);
}

void Board::updateBonuses(const float &delta_time) {
    for (Bonus &bonus: this->m_bonuses) {
        if (!bonus.active) {
            continue;
        }

        bonus.rect.y += BONUS_FALL_SPEED * delta_time;

        if (bonus.rect.y > FIELD_BOTTOM) {
            bonus.active = false;
        }
    }

    this->m_bonuses.erase(std::remove_if(this->m_bonuses.begin(),
                                         this->m_bonuses.end(),
                                         [](const Bonus &bonus) {
                                             return !bonus.active;
                                         }),
                          this->m_bonuses.end());
}

BrickHitResult Board::handleBallCollision(Ball &ball) {
    BrickHitResult result;
    Rect ball_rect = getBallRect(ball);

    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            Brick &brick = this->m_bricks[row][col];

            if (brick.type == BrickType::Empty) {
                continue;
            }

            Rect brick_rect = getBrickRect(row, col);

            if (!intersects(ball_rect, brick_rect)) {
                continue;
            }

            reflectBallFromRect(ball, brick_rect);
            result.hit = true;

            if (brick.type == BrickType::Unbreakable) {
                return result;
            }

            result.score += SCORE_PER_BRICK_HIT;
            brick.health--;

            if (brick.type == BrickType::SpeedUp) {
                result.speed_up_ball = true;
            }

            if (brick.health <= 0) {
                if (brick.has_bonus) {
                    spawnBonus(row, col);
                }

                brick.type = BrickType::Empty;
                brick.health = 0;
                brick.has_bonus = false;
            }

            return result;
        }
    }

    return result;
}

bool Board::collectBonus(const Rect &paddle_rect, BonusType &bonus_type) {
    for (Bonus &bonus: this->m_bonuses) {
        if (!bonus.active) {
            continue;
        }

        if (!intersects(bonus.rect, paddle_rect)) {
            continue;
        }

        bonus_type = bonus.type;
        bonus.active = false;
        return true;
    }

    return false;
}

bool Board::hasRemainingDestructibleBlocks() const {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            const Brick &brick = this->m_bricks[row][col];

            if (brick.type != BrickType::Empty && brick.type != BrickType::Unbreakable) {
                return true;
            }
        }
    }

    return false;
}
