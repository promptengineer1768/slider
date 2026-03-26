#include "slider/board.h"
#include <algorithm>
#include <limits>
#include <vector>

namespace slider {

Board::Board(int size) {
  if (size <= 0) {
    state_ = BoardState();
    move_count_ = 0;
    return;
  }
  const size_t size_u = static_cast<size_t>(size);
  if (size_u > std::numeric_limits<size_t>::max() / size_u) {
    state_ = BoardState();
    move_count_ = 0;
    return;
  }
  const size_t tile_count = size_u * size_u;
  if (tile_count == 0 || tile_count > static_cast<size_t>(std::numeric_limits<int>::max())) {
    state_ = BoardState();
    move_count_ = 0;
    return;
  }

  std::vector<int> tiles(tile_count);
  for (int i = 0; i < static_cast<int>(tile_count) - 1; ++i) {
    tiles[i] = i + 1;
  }
  tiles.back() = 0;
  state_ = BoardState(size, tiles);
  move_count_ = 0;
}

void Board::SetState(const BoardState& state) {
  state_ = state;
  move_count_ = 0;
}

bool Board::Move(Direction dir) {
  int size = state_.GetSize();
  int empty_pos = state_.GetEmptyPos();
  if (size <= 0 || empty_pos < 0) return false;
  int row = empty_pos / size;
  int col = empty_pos % size;

  int target_row = row;
  int target_col = col;

  switch (dir) {
    case Direction::kUp:
      target_row--;
      break;
    case Direction::kDown:
      target_row++;
      break;
    case Direction::kLeft:
      target_col--;
      break;
    case Direction::kRight:
      target_col++;
      break;
  }

  if (target_row < 0 || target_row >= size || target_col < 0 || target_col >= size) {
    return false;
  }

  int target_pos = target_row * size + target_col;
  if (!state_.SwapTiles(empty_pos, target_pos)) {
    return false;
  }
  move_count_++;
  return true;
}

bool Board::MoveTile(int tile_val) {
  if (tile_val == 0) return false;
  auto dir = GetDirectionToMoveTile(tile_val);
  if (dir) {
    return Move(*dir);
  }
  return false;
}

std::vector<Direction> Board::GetValidMoves() const {
  std::vector<Direction> moves;
  int size = state_.GetSize();
  int empty_pos = state_.GetEmptyPos();
  if (size <= 0 || empty_pos < 0) return moves;
  int row = empty_pos / size;
  int col = empty_pos % size;

  if (row > 0) moves.push_back(Direction::kUp);
  if (row < size - 1) moves.push_back(Direction::kDown);
  if (col > 0) moves.push_back(Direction::kLeft);
  if (col < size - 1) moves.push_back(Direction::kRight);

  return moves;
}

std::optional<Direction> Board::GetDirectionToMoveTile(int tile_val) const {
  int size = state_.GetSize();
  if (size <= 0) return std::nullopt;
  int tile_pos = state_.GetTilePos(tile_val);
  if (tile_pos == -1) return std::nullopt;

  int empty_pos = state_.GetEmptyPos();
  int row = empty_pos / size;
  int col = empty_pos % size;

  int tile_row = tile_pos / size;
  int tile_col = tile_pos % size;

  // The returned direction describes how the blank must move to swap with the tile.
  if (tile_row == row - 1 && tile_col == col) return Direction::kUp;
  if (tile_row == row + 1 && tile_col == col) return Direction::kDown;
  if (tile_row == row && tile_col == col - 1) return Direction::kLeft;
  if (tile_row == row && tile_col == col + 1) return Direction::kRight;

  return std::nullopt;
}

} // namespace slider
