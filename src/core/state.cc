#include "slider/state.h"
#include <vector>
#include <algorithm>
#include <sstream>
#include <limits>

namespace slider {

BoardState::BoardState(int size, const std::vector<int>& tiles)
    : size_(size), tiles_(tiles) {
  RebuildTilePositionCache();
}

int BoardState::GetEmptyPos() const {
  return GetTilePos(0);
}

int BoardState::GetTilePos(int tile_value) const {
  const auto it = tile_positions_.find(tile_value);
  if (it == tile_positions_.end()) return -1;
  if (it->second > static_cast<size_t>(std::numeric_limits<int>::max())) return -1;
  return static_cast<int>(it->second);
}

bool BoardState::SwapTiles(int pos_a, int pos_b) {
  if (pos_a < 0 || pos_b < 0) return false;
  const size_t a = static_cast<size_t>(pos_a);
  const size_t b = static_cast<size_t>(pos_b);
  if (a >= tiles_.size() || b >= tiles_.size()) return false;
  std::swap(tiles_[a], tiles_[b]);
  const int tile_a = tiles_[a];
  const int tile_b = tiles_[b];
  tile_positions_[tile_a] = a;
  tile_positions_[tile_b] = b;
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
  const size_t size_u = static_cast<size_t>(size_);
  if (size_u > std::numeric_limits<size_t>::max() / size_u) return false;
  const size_t expected_tiles = size_u * size_u;
  if (tiles_.size() != expected_tiles) return false;
  
  // All tiles 0 to size*size-1 must be present
  std::vector<int> sorted_tiles = tiles_;
  std::sort(sorted_tiles.begin(), sorted_tiles.end());
  for (size_t i = 0; i < sorted_tiles.size(); ++i) {
    if (sorted_tiles[i] != static_cast<int>(i)) return false;
  }

  // Solvability rule for the standard goal state (blank in the bottom-right):
  // odd widths require an even inversion count; even widths require inversion
  // parity to differ from the blank-row parity counted from the bottom.
  
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
    const int empty_pos = GetEmptyPos();
    if (empty_pos < 0) return false;
    const int empty_row_from_top = empty_pos / size_;
    const int blank_row_from_bottom = size_ - empty_row_from_top;
    return (inversions % 2) != (blank_row_from_bottom % 2);
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
  const size_t size_u = static_cast<size_t>(size);
  if (size_u > std::numeric_limits<size_t>::max() / size_u) return BoardState();
  const size_t tile_count = size_u * size_u;
  if (tile_count == 0 || tile_count > static_cast<size_t>(std::numeric_limits<int>::max())) {
    return BoardState();
  }

  std::vector<int> tiles;
  tiles.reserve(tile_count);

  for (size_t i = 0; i < tile_count; ++i) {
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

void BoardState::RebuildTilePositionCache() {
  tile_positions_.clear();
  for (size_t i = 0; i < tiles_.size(); ++i) {
    tile_positions_[tiles_[i]] = i;
  }
}

} // namespace slider
