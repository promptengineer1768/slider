#include "slider/state.h"
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace slider {

BoardState::BoardState(int size, const std::vector<int>& tiles)
    : size_(size), tiles_(tiles) {}

int BoardState::GetEmptyPos() const {
  for (size_t i = 0; i < tiles_.size(); ++i) {
    if (tiles_[i] == 0) return static_cast<int>(i);
  }
  return -1;
}

bool BoardState::SwapTiles(int pos_a, int pos_b) {
  if (pos_a < 0 || pos_b < 0) return false;
  const size_t a = static_cast<size_t>(pos_a);
  const size_t b = static_cast<size_t>(pos_b);
  if (a >= tiles_.size() || b >= tiles_.size()) return false;
  std::swap(tiles_[a], tiles_[b]);
  return true;
}

bool BoardState::IsSolved() const {
  if (tiles_.empty()) return false;
  // A solved state is 1, 2, ..., size*size-1, 0
  for (size_t i = 0; i + 1 < tiles_.size(); ++i) {
    if (tiles_[i] != static_cast<int>(i) + 1) return false;
  }
  return tiles_.back() == 0;
}

bool BoardState::IsValid() const {
  if (size_ <= 0) return false;
  if (tiles_.size() != static_cast<size_t>(size_ * size_)) return false;
  
  // All tiles 0 to size*size-1 must be present
  std::vector<int> sorted_tiles = tiles_;
  std::sort(sorted_tiles.begin(), sorted_tiles.end());
  for (size_t i = 0; i < sorted_tiles.size(); ++i) {
    if (sorted_tiles[i] != static_cast<int>(i)) return false;
  }

  // Solvability check for sliding puzzle
  // For even width, row of empty tile from bottom (1st from bottom = 0) must have parities... 
  // For simplicity, I'll rely on the fact that if we generate from moves it's always solvable.
  // But let's check properly:
  // An N*N puzzle is solvable iff:
  // If N is odd, number of inversions is even.
  // If N is even, number of inversions + row of empty from bottom (1st = 0) is even? 
  // Let's check Wikipedia. 
  // Wikipedia: 
  // a) width is odd, then inversions is even.
  // b) width is even:
  //   - blank on even row from bottom (0, 2, 4...) and inversions is odd.
  //   - blank on odd row from bottom (1, 3, 5...) and inversions is even.
  
  int inversions = 0;
  for (size_t i = 0; i < tiles_.size(); ++i) {
    if (tiles_[i] == 0) continue;
    for (size_t j = i + 1; j < tiles_.size(); ++j) {
      if (tiles_[j] != 0 && tiles_[i] > tiles_[j]) {
        inversions++;
      }
    }
  }

  if (size_ % 2 != 0) {
    return inversions % 2 == 0;
  } else {
    int empty_row_from_top = GetEmptyPos() / size_;
    int empty_row_from_bottom = size_ - empty_row_from_top;
    if (empty_row_from_bottom % 2 != 0) {
      return inversions % 2 == 0;
    } else {
      return inversions % 2 != 0;
    }
  }
}

bool BoardState::operator==(const BoardState& other) const {
  return size_ == other.size_ && tiles_ == other.tiles_;
}

bool BoardState::operator<(const BoardState& other) const {
  if (size_ != other.size_) return size_ < other.size_;
  return tiles_ < other.tiles_;
}

std::string BoardState::Serialize() const {
  std::stringstream ss;
  ss << size_ << " ";
  for (int t : tiles_) ss << t << " ";
  return ss.str();
}

BoardState BoardState::Deserialize(const std::string& data) {
  std::stringstream ss(data);
  int size = 0;
  ss >> size;
  if (size <= 0) return BoardState();
  const int tile_count = size * size;
  if (tile_count <= 0) return BoardState();

  std::vector<int> tiles;
  tiles.reserve(static_cast<size_t>(tile_count));

  for (int i = 0; i < tile_count; ++i) {
    int v = 0;
    if (!(ss >> v)) {
      return BoardState();
    }
    tiles.push_back(v);
  }

  BoardState state(size, tiles);
  if (!state.IsValid()) return BoardState();
  return state;
}

} // namespace slider
