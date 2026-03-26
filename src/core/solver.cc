#include "slider/solver.h"
#include <queue>
#include <map>
#include <vector>
#include <cmath>
#include <set>
#include <algorithm>
#include <utility>

namespace slider {

namespace {

int GetManhattanDistance(const BoardState& state) {
  int size = state.GetSize();
  const auto& tiles = state.GetTiles();
  int distance = 0;
  for (size_t i = 0; i < tiles.size(); ++i) {
    if (tiles[i] == 0) continue;
    int target_val = tiles[i];
    int target_pos = target_val - 1;
    int target_row = target_pos / size;
    int target_col = target_pos % size;
    int row = static_cast<int>(i) / size;
    int col = static_cast<int>(i) % size;
    distance += std::abs(row - target_row) + std::abs(col - target_col);
  }
  return distance;
}

int GetLinearConflict(const BoardState& state) {
  int size = state.GetSize();
  const auto& tiles = state.GetTiles();
  int conflict = 0;

  // Row conflicts
  for (int r = 0; r < size; ++r) {
    for (int c1 = 0; c1 < size; ++c1) {
      int t1 = tiles[r * size + c1];
      if (t1 == 0) continue;
      if ((t1 - 1) / size != r) continue; 

      for (int c2 = c1 + 1; c2 < size; ++c2) {
        int t2 = tiles[r * size + c2];
        if (t2 == 0) continue;
        if ((t2 - 1) / size != r) continue;

        if (t1 > t2) { 
          conflict += 2;
        }
      }
    }
  }

  // Column conflicts
  for (int c = 0; c < size; ++c) {
    for (int r1 = 0; r1 < size; ++r1) {
      int t1 = tiles[r1 * size + c];
      if (t1 == 0) continue;
      if ((t1 - 1) % size != c) continue; 

      for (int r2 = r1 + 1; r2 < size; ++r2) {
        int t2 = tiles[r2 * size + c];
        if (t2 == 0) continue;
        if ((t2 - 1) % size != c) continue;

        if (t1 > t2) { 
          conflict += 2;
        }
      }
    }
  }

  return conflict;
}

int GetHeuristic(const BoardState& state) {
  return GetManhattanDistance(state) + GetLinearConflict(state);
}

struct Node {
  BoardState state;
  int g = 0; 
  int h = 0; 

  bool operator>(const Node& other) const {
    return (g + h) > (other.g + other.h);
  }
};

std::vector<Direction> GetValidMovesFromEmpty(int size, int empty_pos) {
  std::vector<Direction> moves;
  const int row = empty_pos / size;
  const int col = empty_pos % size;

  if (row > 0) moves.push_back(Direction::kUp);
  if (row < size - 1) moves.push_back(Direction::kDown);
  if (col > 0) moves.push_back(Direction::kLeft);
  if (col < size - 1) moves.push_back(Direction::kRight);

  return moves;
}

bool TryApplyMove(BoardState* state, Direction dir) {
  if (!state) return false;
  const int size = state->GetSize();
  const int empty_pos = state->GetEmptyPos();
  if (size <= 0 || empty_pos < 0) return false;

  const int row = empty_pos / size;
  const int col = empty_pos % size;

  int target_row = row;
  int target_col = col;

  switch (dir) {
    case Direction::kUp:    target_row--; break;
    case Direction::kDown:  target_row++; break;
    case Direction::kLeft:  target_col--; break;
    case Direction::kRight: target_col++; break;
  }

  if (target_row < 0 || target_row >= size || target_col < 0 || target_col >= size) {
    return false;
  }

  const int target_pos = target_row * size + target_col;
  return state->SwapTiles(empty_pos, target_pos);
}

std::vector<Direction> ReconstructPath(
    const BoardState& start,
    BoardState current,
    const std::map<BoardState, std::pair<BoardState, Direction>>& came_from) {
  std::vector<Direction> path;

  while (!(current == start)) {
    auto it = came_from.find(current);
    if (it == came_from.end()) {
      // No path (shouldn't happen if called correctly).
      return {};
    }
    path.push_back(it->second.second);
    current = it->second.first;
  }

  std::reverse(path.begin(), path.end());
  return path;
}

} // namespace

Solution Solver::Solve(const BoardState& start_state) {
  return Solve(start_state, SolverOptions{});
}

Solution Solver::Solve(const BoardState& start_state, const SolverOptions& options) {
  if (!start_state.IsValid()) return {{}, false};
  if (start_state.IsSolved()) return {{}, true};

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
  std::map<BoardState, int> g_score;
  std::map<BoardState, std::pair<BoardState, Direction>> came_from;

  open_set.push({start_state, 0, GetHeuristic(start_state)});
  g_score[start_state] = 0;

  const int nodes_limit = (options.nodes_limit > 0) ? options.nodes_limit : 100000;
  int nodes_checked = 0;

  while (!open_set.empty()) {
    Node current = open_set.top();
    open_set.pop();

    if (current.state.IsSolved()) {
      return {ReconstructPath(start_state, current.state, came_from), true};
    }

    if (nodes_checked++ > nodes_limit) break;

    const int size = current.state.GetSize();
    const int empty_pos = current.state.GetEmptyPos();
    if (size <= 0 || empty_pos < 0) continue;
    auto valid_moves = GetValidMovesFromEmpty(size, empty_pos);

    for (Direction dir : valid_moves) {
      BoardState next_state = current.state;
      if (!TryApplyMove(&next_state, dir)) {
        continue;
      }

      const int tentative_g = current.g + 1;
      auto it = g_score.find(next_state);
      if (it == g_score.end() || tentative_g < it->second) {
        g_score[next_state] = tentative_g;
        came_from[next_state] = std::make_pair(current.state, dir);
        open_set.push({next_state, tentative_g, GetHeuristic(next_state)});
      }
    }
  }

  return {{}, false};
}

Solution Solver::SolveNSteps(const BoardState& start_state, int n) {
  Solution full = Solve(start_state);
  if (full.success) {
    if (full.moves.size() > static_cast<size_t>(n)) {
      full.moves.resize(n);
    }
    return full;
  }
  return {{}, false};
}

} // namespace slider
