#include "Board.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <queue>

GameElement::GameElement(const int &color_index) {
    this->m_color_index = color_index;
    this->m_selected = false;
}

int GameElement::activate(Board &board, const DestroyedCell &source_cell) {
    (void) board;
    (void) source_cell;
    return 0;
}

bool GameElement::isBonus() const {
    return false;
}

int GameElement::getColorIndex() const {
    return this->m_color_index;
}

void GameElement::setColorIndex(const int &color_index) {
    this->m_color_index = color_index;
}

bool GameElement::isSelected() const {
    return this->m_selected;
}

void GameElement::setSelected(const bool &selected) {
    this->m_selected = selected;
}

StandardElement::StandardElement(const int &color_index) : GameElement(color_index) {
}

RepaintBonusElement::RepaintBonusElement(const int &color_index) : GameElement(color_index) {
}

bool RepaintBonusElement::isBonus() const {
    return true;
}

int RepaintBonusElement::activate(Board &board, const DestroyedCell &source_cell) {
    std::pair<int, int> landing_cell;

    if (!board.pickRandomNonEmptyCellInRadius(source_cell.row, source_cell.col, BONUS_RADIUS, landing_cell)) {
        return 0;
    }

    int landing_row = landing_cell.first;
    int landing_col = landing_cell.second;

    board.setColorIndex(landing_row, landing_col, this->m_color_index);

    std::vector<std::pair<int, int> > candidates;

    for (int row = landing_row - BONUS_RADIUS; row <= landing_row + BONUS_RADIUS; row++) {
        for (int col = landing_col - BONUS_RADIUS; col <= landing_col + BONUS_RADIUS; col++) {
            if (!Board::isValidCell(row, col)) {
                continue;
            }
            if (row == landing_row && col == landing_col) {
                continue;
            }
            if (board.isEmpty(row, col)) {
                continue;
            }
            if (Board::isDirectNeighbor(landing_row, landing_col, row, col)) {
                continue;
            }

            candidates.push_back({row, col});
        }
    }

    board.shuffleCells(candidates);

    constexpr int REPAINT_EXTRA_CELLS = 2;
    int cells_to_repaint = std::min(REPAINT_EXTRA_CELLS, static_cast<int>(candidates.size()));

    for (int i = 0; i < cells_to_repaint; i++) {
        int row = candidates[i].first;
        int col = candidates[i].second;

        board.setColorIndex(row, col, this->m_color_index);
    }

    return 0;
}

BombBonusElement::BombBonusElement(const int &color_index) : GameElement(color_index) {
}

bool BombBonusElement::isBonus() const {
    return true;
}

int BombBonusElement::activate(Board &board, const DestroyedCell &source_cell) {
    std::pair<int, int> landing_cell;

    if (!board.pickRandomNonEmptyCellInRadius(source_cell.row, source_cell.col, BONUS_RADIUS, landing_cell)) {
        return 0;
    }

    std::vector<std::pair<int, int> > targets;
    std::vector<std::pair<int, int> > other_cells;

    targets.push_back(landing_cell);

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            if (board.isEmpty(row, col)) {
                continue;
            }
            if (row == landing_cell.first && col == landing_cell.second) {
                continue;
            }

            other_cells.push_back({row, col});
        }
    }

    board.shuffleCells(other_cells);

    constexpr int BOMB_ADDITIONAL_TARGETS = 4;
    int additional_targets = std::min(BOMB_ADDITIONAL_TARGETS, static_cast<int>(other_cells.size()));

    for (int i = 0; i < additional_targets; i++) {
        targets.push_back(other_cells[i]);
    }

    for (auto &target: targets) {
        board.clearCell(target.first, target.second);
    }

    return static_cast<int>(targets.size());
}

std::unique_ptr<GameElement> ElementFactory::createStandard(const int &color_index) {
    return std::make_unique<StandardElement>(color_index);
}

std::unique_ptr<GameElement> ElementFactory::createRandomBonus(const int &color_index, const int &bonus_type) {
    if (bonus_type == 0) {
        return std::make_unique<RepaintBonusElement>(color_index);
    }

    return std::make_unique<BombBonusElement>(color_index);
}

Board::Board() {
    this->m_rng.seed(static_cast<unsigned int>(std::time(nullptr)));
}

void Board::init() {
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            this->m_cells[row][col] = ElementFactory::createStandard(randomColorIndex());
        }
    }
}

const GameElement *Board::getCell(const int &row, const int &col) const {
    if (!isValidCell(row, col)) {
        return nullptr;
    }

    return this->m_cells[row][col].get();
}

GameElement *Board::getCell(const int &row, const int &col) {
    if (!isValidCell(row, col)) {
        return nullptr;
    }

    return this->m_cells[row][col].get();
}

int Board::getColorIndex(const int &row, const int &col) const {
    const GameElement *cell = getCell(row, col);

    if (!cell) {
        return EMPTY_CELL;
    }

    return cell->getColorIndex();
}

bool Board::isEmpty(const int &row, const int &col) const {
    return getColorIndex(row, col) == EMPTY_CELL;
}

void Board::setColorIndex(const int &row, const int &col, const int &color_index) {
    if (!isValidCell(row, col)) {
        return;
    }

    if (!this->m_cells[row][col]) {
        this->m_cells[row][col] = ElementFactory::createStandard(color_index);
        return;
    }

    this->m_cells[row][col]->setColorIndex(color_index);
    this->m_cells[row][col]->setSelected(false);
}

void Board::clearCell(const int &row, const int &col) {
    if (!isValidCell(row, col)) {
        return;
    }

    this->m_cells[row][col].reset();
}

void Board::setSelected(const int &row, const int &col, const bool &selected) {
    GameElement *cell = getCell(row, col);

    if (!cell) {
        return;
    }

    cell->setSelected(selected);
}

void Board::swapCells(const int &row_1, const int &col_1, const int &row_2, const int &col_2) {
    std::swap(this->m_cells[row_1][col_1], this->m_cells[row_2][col_2]);
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

void Board::shuffleCells(std::vector<std::pair<int, int> > &cells) {
    std::shuffle(cells.begin(), cells.end(), this->m_rng);
}

void Board::collectConnectedGroup(const int &start_row,
                                  const int &start_col,
                                  BoolGrid &visited,
                                  std::vector<std::pair<int, int> > &group) {
    const int target_color = getColorIndex(start_row, start_col);

    std::queue<std::pair<int, int> > cells;
    cells.push({start_row, start_col});
    visited[start_row][start_col] = true;

    constexpr int d_rows[4] = {-1, 1, 0, 0};
    constexpr int d_cols[4] = {0, 0, -1, 1};

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
            if (getColorIndex(next_row, next_col) != target_color) {
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
            if (isEmpty(row, col)) {
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

            destroyed_cells.push_back({row, col, getColorIndex(row, col)});
            clearCell(row, col);
            destroyed_count++;
        }
    }

    if (!allow_bonuses) {
        return destroyed_count;
    }

    int bonus_destroyed_count = 0;
    for (auto &destroyed_cell: destroyed_cells) {
        if (randomChance(BONUS_CHANCE_PERCENT)) {
            std::unique_ptr<GameElement> bonus = ElementFactory::createRandomBonus(destroyed_cell.original_color,
                                                                                   randomInt(0, 1));
            bonus_destroyed_count += bonus->activate(*this, destroyed_cell);
        }
    }

    return destroyed_count + bonus_destroyed_count;
}

void Board::applyGravityAndRefill() {
    for (int col = 0; col < BOARD_COLS; col++) {
        int write_row = BOARD_ROWS - 1;

        for (int row = BOARD_ROWS - 1; row >= 0; row--) {
            if (isEmpty(row, col)) {
                continue;
            }

            if (write_row != row) {
                this->m_cells[write_row][col] = std::move(this->m_cells[row][col]);
            }

            this->m_cells[write_row][col]->setSelected(false);
            write_row--;
        }

        for (int row = write_row; row >= 0; row--) {
            this->m_cells[row][col] = ElementFactory::createStandard(randomColorIndex());
        }
    }
}

int Board::processCascades(const bool &allow_bonuses) {
    int total_destroyed = 0;
    int safety_counter = 0;

    constexpr int MAX_CASCADE_STEPS = 100;

    while (safety_counter < MAX_CASCADE_STEPS) {
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
            if (isEmpty(row, col)) {
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
