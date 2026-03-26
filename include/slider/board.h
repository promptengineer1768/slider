#ifndef SLIDER_CORE_BOARD_H_
#define SLIDER_CORE_BOARD_H_

#include "slider/state.h"
#include <optional>
#include <vector>

namespace slider {

enum class Direction {
  kUp,
  kDown,
  kLeft,
  kRight
};

class Board {
 public:
  Board() = default;
  explicit Board(int size);

  int GetSize() const { return state_.GetSize(); }
  const BoardState& GetState() const { return state_; }
  void SetState(const BoardState& state);

  bool Move(Direction dir);
  bool MoveTile(int tile_val); // Move tile if it's adjacent to empty

  bool IsSolved() const { return state_.IsSolved(); }
  int GetMoveCount() const { return move_count_; }
  void ResetMoveCount() { move_count_ = 0; }

  // Returns the legal slide directions from the current empty-cell position.
  std::vector<Direction> GetValidMoves() const;
  // Maps a tile value to the direction the empty cell must move to slide it.
  std::optional<Direction> GetDirectionToMoveTile(int tile_val) const;

 private:
  BoardState state_;
  int move_count_ = 0;
};

} // namespace slider

#endif // SLIDER_CORE_BOARD_H_
