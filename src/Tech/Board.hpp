#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <random>
#include <utility>
#include <vector>

#include "Containers/Config.hpp"
#include "Containers/Types.hpp"

class Board {
private:
    using CellGrid = std::array<std::array<Cell, BOARD_COLS>, BOARD_ROWS>;
    using BoolGrid = std::array<std::array<bool, BOARD_COLS>, BOARD_ROWS>;

    CellGrid m_cells;
    std::mt19937 m_rng;

    int randomInt(const int &min_value, const int &max_value);
    bool randomChance(const int &percent);
    int randomColorIndex();

    static bool isDirectNeighbor(const int &row_1, const int &col_1, const int &row_2, const int &col_2);

    void collectConnectedGroup(const int &start_row,
                               const int &start_col,
                               BoolGrid &visited,
                               std::vector<std::pair<int, int> > &group);

    int destroyMatchedGroups(const bool &allow_bonuses);
    void applyGravityAndRefill();

    std::vector<std::pair<int, int> > getNonEmptyCellsInRadius(const int &center_row,
                                                                const int &center_col,
                                                                const int &radius);

    bool pickRandomNonEmptyCellInRadius(const int &center_row,
                                         const int &center_col,
                                         const int &radius,
                                         std::pair<int, int> &result);

    int applyRandomBonus(const DestroyedCell &source_cell);
    int applyRepaintBonus(const DestroyedCell &source_cell);
    int applyBombBonus(const DestroyedCell &source_cell);

public:
    Board();

    void init();

    const Cell &getCell(const int &row, const int &col) const {
        return this->m_cells[row][col];
    }

    Cell &getCell(const int &row, const int &col) {
        return this->m_cells[row][col];
    }

    void setSelected(const int &row, const int &col, const bool &selected) {
        if (!isValidCell(row, col)) {
            return;
        }
        this->m_cells[row][col].selected = selected;
    }

    void swapCells(const int &row_1, const int &col_1, const int &row_2, const int &col_2);

    int processCascades(const bool &allow_bonuses);

    static bool isValidCell(const int &row, const int &col);
    static bool areAdjacent(const int &row_1, const int &col_1, const int &row_2, const int &col_2);
};

#endif //BOARD_H
