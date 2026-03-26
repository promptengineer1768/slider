#ifndef SLIDER_CORE_STATE_H_
#define SLIDER_CORE_STATE_H_

#include <vector>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace slider {

class BoardState {
 public:
  // Represents a square sliding-puzzle board in row-major tile order.
  BoardState() = default;
  // Constructs a board state from the provided tiles.
  BoardState(int size, const std::vector<int>& tiles);

  int GetSize() const { return size_; }
  const std::vector<int>& GetTiles() const { return tiles_; }
  // Returns the position of the blank tile, or -1 if it cannot be found.
  int GetEmptyPos() const;
  // Returns the position of a tile value, or -1 if the value is missing.
  int GetTilePos(int tile_value) const;
  bool SwapTiles(int pos_a, int pos_b);

  bool IsSolved() const;
  bool IsValid() const;

  bool operator==(const BoardState& other) const;
  bool operator<(const BoardState& other) const; // For std::set/std::map

  std::string Serialize() const;
  static BoardState Deserialize(const std::string& data);

 private:
  void RebuildTilePositionCache();

  int size_ = 0;
  std::vector<int> tiles_;
  mutable std::unordered_map<int, size_t> tile_positions_;
};

} // namespace slider

#endif // SLIDER_CORE_STATE_H_
