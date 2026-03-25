#include "slider/scrambler.h"
#include <random>
#include <algorithm>
#include <vector>

namespace slider {

std::vector<Direction> Scrambler::Scramble(Board& board, int max_moves) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::vector<Direction> applied_moves;

  Direction last_move = Direction::kUp; // Doesn't matter, just to avoid immediate undo
  bool has_last = false;

  for (int i = 0; i < max_moves; ++i) {
    auto valid_moves = board.GetValidMoves();
    if (valid_moves.empty()) break;

    // Filter out moves that undo the last move
    if (has_last) {
        Direction undo_move;
        switch (last_move) {
            case Direction::kUp: undo_move = Direction::kDown; break;
            case Direction::kDown: undo_move = Direction::kUp; break;
            case Direction::kLeft: undo_move = Direction::kRight; break;
            case Direction::kRight: undo_move = Direction::kLeft; break;
        }
        auto it = std::find(valid_moves.begin(), valid_moves.end(), undo_move);
        if (it != valid_moves.end() && valid_moves.size() > 1) {
            valid_moves.erase(it);
        }
    }

    std::uniform_int_distribution<> dis(0, static_cast<int>(valid_moves.size() - 1));
    Direction move = valid_moves[dis(gen)];

    if (board.Move(move)) {
      applied_moves.push_back(move);
      last_move = move;
      has_last = true;
    }
  }

  return applied_moves;
}

} // namespace slider
