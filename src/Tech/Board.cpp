#include "Board.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <queue>

Board::Board() {
    this->m_rng.seed(static_cast<unsigned int>(std::time(nullptr)));
}

void Board::init() {
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            this->m_cells[row][col].color_index = randomColorIndex();
            this->m_cells[row][col].selected = false;
        }
    }
}

void Board::swapCells(const int &row_1, const int &col_1, const int &row_2, const int &col_2) {
    std::swap(this->m_cells[row_1][col_1].color_index, this->m_cells[row_2][col_2].color_index);
}

bool Board::isValidCell(const int &row, const int &col) {
    return row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS;
}

bool Board::isDirectNeighbor(const int &row_1, const int &col_1, const int &row_2, const int &col_2) {
    int row_diff = std::abs(row_1 - row_2);
    int col_diff = std::abs(col_1 - col_2);

    return row_diff + col_diff == 1;
}

bool Board::areAdjacent(const int &row_1, const int &col_1, const int &row_2, const int &col_2) {
    return isDirectNeighbor(row_1, col_1, row_2, col_2);
}

int Board::randomInt(const int &min_value, const int &max_value) {
    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(this->m_rng);
}

bool Board::randomChance(const int &percent) {
    return randomInt(1, 100) <= percent;
}

int Board::randomColorIndex() {
    return randomInt(0, COLOR_COUNT - 1);
}

void Board::collectConnectedGroup(const int &start_row,
                                  const int &start_col,
                                  BoolGrid &visited,
                                  std::vector<std::pair<int, int> > &group) {
    const int target_color = this->m_cells[start_row][start_col].color_index;

    std::queue<std::pair<int, int> > cells;
    cells.push({start_row, start_col});
    visited[start_row][start_col] = true;

    const int d_rows[4] = {-1, 1, 0, 0};
    const int d_cols[4] = {0, 0, -1, 1};

    while (!cells.empty()) {
        auto [row, col] = cells.front();
        cells.pop();

        group.push_back({row, col});

        for (int direction = 0; direction < 4; direction++) {
            int next_row = row + d_rows[direction];
            int next_col = col + d_cols[direction];

            if (!isValidCell(next_row, next_col)) {
                continue;
            }
            if (visited[next_row][next_col]) {
                continue;
            }
            if (this->m_cells[next_row][next_col].color_index != target_color) {
                continue;
            }

            visited[next_row][next_col] = true;
            cells.push({next_row, next_col});
        }
    }
}

int Board::destroyMatchedGroups(const bool &allow_bonuses) {
    BoolGrid visited{};
    BoolGrid should_destroy{};
    std::vector<DestroyedCell> destroyed_cells;
    int destroyed_count = 0;

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            if (visited[row][col]) {
                continue;
            }
            if (this->m_cells[row][col].color_index == EMPTY_CELL) {
                visited[row][col] = true;
                continue;
            }

            std::vector<std::pair<int, int> > group;
            collectConnectedGroup(row, col, visited, group);

            if (group.size() >= 3) {
                for (auto &cell: group) {
                    should_destroy[cell.first][cell.second] = true;
                }
            }
        }
    }

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            if (!should_destroy[row][col]) {
                continue;
            }

            destroyed_cells.push_back({row, col, this->m_cells[row][col].color_index});
            this->m_cells[row][col].color_index = EMPTY_CELL;
            this->m_cells[row][col].selected = false;
            destroyed_count++;
        }
    }

    if (!allow_bonuses) {
        return destroyed_count;
    }

    int bonus_destroyed_count = 0;
    for (auto &destroyed_cell: destroyed_cells) {
        if (randomChance(BONUS_CHANCE_PERCENT)) {
            bonus_destroyed_count += applyRandomBonus(destroyed_cell);
        }
    }

    return destroyed_count + bonus_destroyed_count;
}

void Board::applyGravityAndRefill() {
    for (int col = 0; col < BOARD_COLS; col++) {
        int write_row = BOARD_ROWS - 1;

        for (int row = BOARD_ROWS - 1; row >= 0; row--) {
            if (this->m_cells[row][col].color_index == EMPTY_CELL) {
                continue;
            }

            this->m_cells[write_row][col].color_index = this->m_cells[row][col].color_index;
            this->m_cells[write_row][col].selected = false;

            if (write_row != row) {
                this->m_cells[row][col].color_index = EMPTY_CELL;
                this->m_cells[row][col].selected = false;
            }

            write_row--;
        }

        for (int row = write_row; row >= 0; row--) {
            this->m_cells[row][col].color_index = randomColorIndex();
            this->m_cells[row][col].selected = false;
        }
    }
}

int Board::processCascades(const bool &allow_bonuses) {
    int total_destroyed = 0;
    int safety_counter = 0;

    while (safety_counter < 100) {
        int destroyed_now = destroyMatchedGroups(allow_bonuses);

        if (destroyed_now == 0) {
            break;
        }

        total_destroyed += destroyed_now;
        applyGravityAndRefill();
        safety_counter++;
    }

    return total_destroyed;
}

std::vector<std::pair<int, int> > Board::getNonEmptyCellsInRadius(const int &center_row,
                                                                   const int &center_col,
                                                                   const int &radius) {
    std::vector<std::pair<int, int> > result;

    for (int row = center_row - radius; row <= center_row + radius; row++) {
        for (int col = center_col - radius; col <= center_col + radius; col++) {
            if (!isValidCell(row, col)) {
                continue;
            }
            if (this->m_cells[row][col].color_index == EMPTY_CELL) {
                continue;
            }

            result.push_back({row, col});
        }
    }

    return result;
}

bool Board::pickRandomNonEmptyCellInRadius(const int &center_row,
                                            const int &center_col,
                                            const int &radius,
                                            std::pair<int, int> &result) {
    std::vector<std::pair<int, int> > cells = getNonEmptyCellsInRadius(center_row, center_col, radius);

    if (cells.empty()) {
        return false;
    }

    result = cells[randomInt(0, static_cast<int>(cells.size()) - 1)];
    return true;
}

int Board::applyRandomBonus(const DestroyedCell &source_cell) {
    BonusType type = randomInt(0, 1) == 0 ? BonusType::Repaint : BonusType::Bomb;

    if (type == BonusType::Repaint) {
        return applyRepaintBonus(source_cell);
    }
    return applyBombBonus(source_cell);
}

int Board::applyRepaintBonus(const DestroyedCell &source_cell) {
    std::pair<int, int> landing_cell;

    if (!pickRandomNonEmptyCellInRadius(source_cell.row, source_cell.col, BONUS_RADIUS, landing_cell)) {
        return 0;
    }

    int landing_row = landing_cell.first;
    int landing_col = landing_cell.second;

    this->m_cells[landing_row][landing_col].color_index = source_cell.original_color;
    this->m_cells[landing_row][landing_col].selected = false;

    std::vector<std::pair<int, int> > candidates;

    for (int row = landing_row - BONUS_RADIUS; row <= landing_row + BONUS_RADIUS; row++) {
        for (int col = landing_col - BONUS_RADIUS; col <= landing_col + BONUS_RADIUS; col++) {
            if (!isValidCell(row, col)) {
                continue;
            }
            if (row == landing_row && col == landing_col) {
                continue;
            }
            if (this->m_cells[row][col].color_index == EMPTY_CELL) {
                continue;
            }
            if (isDirectNeighbor(landing_row, landing_col, row, col)) {
                continue;
            }

            candidates.push_back({row, col});
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), this->m_rng);

    int cells_to_repaint = std::min(2, static_cast<int>(candidates.size()));

    for (int i = 0; i < cells_to_repaint; i++) {
        int row = candidates[i].first;
        int col = candidates[i].second;

        this->m_cells[row][col].color_index = source_cell.original_color;
        this->m_cells[row][col].selected = false;
    }

    return 0;
}

int Board::applyBombBonus(const DestroyedCell &source_cell) {
    std::pair<int, int> landing_cell;

    if (!pickRandomNonEmptyCellInRadius(source_cell.row, source_cell.col, BONUS_RADIUS, landing_cell)) {
        return 0;
    }

    std::vector<std::pair<int, int> > targets;
    std::vector<std::pair<int, int> > other_cells;

    targets.push_back(landing_cell);

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            if (this->m_cells[row][col].color_index == EMPTY_CELL) {
                continue;
            }
            if (row == landing_cell.first && col == landing_cell.second) {
                continue;
            }

            other_cells.push_back({row, col});
        }
    }

    std::shuffle(other_cells.begin(), other_cells.end(), this->m_rng);

    int additional_targets = std::min(4, static_cast<int>(other_cells.size()));
    for (int i = 0; i < additional_targets; i++) {
        targets.push_back(other_cells[i]);
    }

    for (auto &target: targets) {
        this->m_cells[target.first][target.second].color_index = EMPTY_CELL;
        this->m_cells[target.first][target.second].selected = false;
    }

    return static_cast<int>(targets.size());
}
