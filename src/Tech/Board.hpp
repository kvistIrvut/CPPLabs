#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "Containers/Config.hpp"
#include "Containers/Types.hpp"

class Board;

class GameElement {
protected:
    int m_color_index;
    bool m_selected;

public:
    explicit GameElement(const int &color_index);
    virtual ~GameElement() = default;

    virtual int activate(Board &board, const DestroyedCell &source_cell);
    virtual bool isBonus() const;

    int getColorIndex() const;
    void setColorIndex(const int &color_index);

    bool isSelected() const;
    void setSelected(const bool &selected);
};

class StandardElement final : public GameElement {
public:
    explicit StandardElement(const int &color_index);
};

class RepaintBonusElement final : public GameElement {
public:
    explicit RepaintBonusElement(const int &color_index);

    bool isBonus() const override;
    int activate(Board &board, const DestroyedCell &source_cell) override;
};

class BombBonusElement final : public GameElement {
public:
    explicit BombBonusElement(const int &color_index);

    bool isBonus() const override;
    int activate(Board &board, const DestroyedCell &source_cell) override;
};

class ElementFactory {
public:
    static std::unique_ptr<GameElement> createStandard(const int &color_index);
    static std::unique_ptr<GameElement> createRandomBonus(const int &color_index, const int &bonus_type);
};

class Board {
private:
    using ElementPtr = std::unique_ptr<GameElement>;
    using CellGrid = std::array<std::array<ElementPtr, BOARD_COLS>, BOARD_ROWS>;
    using BoolGrid = std::array<std::array<bool, BOARD_COLS>, BOARD_ROWS>;

    CellGrid m_cells;
    std::mt19937 m_rng;

    bool randomChance(const int &percent);
    int randomColorIndex();

    void collectConnectedGroup(const int &start_row,
                               const int &start_col,
                               BoolGrid &visited,
                               std::vector<std::pair<int, int> > &group);

    int destroyMatchedGroups(const bool &allow_bonuses);
    void applyGravityAndRefill();

public:
    Board();

    void init();

    const GameElement *getCell(const int &row, const int &col) const;
    GameElement *getCell(const int &row, const int &col);

    int getColorIndex(const int &row, const int &col) const;
    bool isEmpty(const int &row, const int &col) const;

    void setColorIndex(const int &row, const int &col, const int &color_index);
    void clearCell(const int &row, const int &col);
    void setSelected(const int &row, const int &col, const bool &selected);
    void swapCells(const int &row_1, const int &col_1, const int &row_2, const int &col_2);

    int processCascades(const bool &allow_bonuses);

    int randomInt(const int &min_value, const int &max_value);
    void shuffleCells(std::vector<std::pair<int, int> > &cells);

    std::vector<std::pair<int, int> > getNonEmptyCellsInRadius(const int &center_row,
                                                                const int &center_col,
                                                                const int &radius);

    bool pickRandomNonEmptyCellInRadius(const int &center_row,
                                         const int &center_col,
                                         const int &radius,
                                         std::pair<int, int> &result);

    static bool isValidCell(const int &row, const int &col);
    static bool isDirectNeighbor(const int &row_1, const int &col_1, const int &row_2, const int &col_2);
    static bool areAdjacent(const int &row_1, const int &col_1, const int &row_2, const int &col_2);
};

#endif //BOARD_H
