#include "slider/board.h"
#include <algorithm>
#include <vector>

namespace slider {

Board::Board(int size) {
  std::vector<int> tiles(size * size);
  for (int i = 0; i < size * size - 1; ++i) {
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
  std::vector<int> tiles = state_.GetTiles();
  std::swap(tiles[empty_pos], tiles[target_pos]);
  state_ = BoardState(size, tiles);
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
  const auto& tiles = state_.GetTiles();
  int tile_pos = -1;
  for (int i = 0; i < tiles.size(); ++i) {
    if (tiles[i] == tile_val) {
      tile_pos = i;
      break;
    }
  }
  if (tile_pos == -1) return std::nullopt;

  int empty_pos = state_.GetEmptyPos();
  int row = empty_pos / size;
  int col = empty_pos % size;

  int tile_row = tile_pos / size;
  int tile_col = tile_pos % size;

  if (tile_row == row - 1 && tile_col == col) return Direction::kUp;
  if (tile_row == row + 1 && tile_col == col) return Direction::kDown;
  if (tile_row == row && tile_col == col - 1) return Direction::kLeft;
  if (tile_row == row && tile_col == col + 1) return Direction::kRight;

  return std::nullopt;
}

} // namespace slider
