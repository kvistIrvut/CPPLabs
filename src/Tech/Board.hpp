#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <random>
#include <vector>

#include "Containers/Config.hpp"
#include "Containers/Types.hpp"

class Board {
private:
    using BrickGrid = std::array<std::array<Brick, BRICK_COLS>, BRICK_ROWS>;

    BrickGrid m_bricks;
    std::vector<Bonus> m_bonuses;
    std::mt19937 m_rng;

    int randomInt(const int &min_value, const int &max_value);
    float randomFloat(const float &min_value, const float &max_value);
    bool randomChance(const int &percent);
    BonusType randomBonusType();

    static Rect getBrickRect(const int &row, const int &col);
    static Rect getBallRect(const Ball &ball);
    static bool intersects(const Rect &first, const Rect &second);
    static float getOverlapX(const Rect &first, const Rect &second);
    static float getOverlapY(const Rect &first, const Rect &second);

    Brick createBrick(const BrickType &type);
    void spawnBonus(const int &row, const int &col);
    void reflectBallFromRect(Ball &ball, const Rect &rect) const;

public:
    Board();

    void init();
    void updateBonuses(const float &delta_time);

    const Brick &getBrick(const int &row, const int &col) const {
        return this->m_bricks[row][col];
    }

    const std::vector<Bonus> &getBonuses() const {
        return this->m_bonuses;
    }

    BrickHitResult handleBallCollision(Ball &ball);
    bool collectBonus(const Rect &paddle_rect, BonusType &bonus_type);
    bool hasRemainingDestructibleBlocks() const;

    static bool isValidBrick(const int &row, const int &col);
    static Rect getBrickScreenRect(const int &row, const int &col);
};

#endif //BOARD_H
