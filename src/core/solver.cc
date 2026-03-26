#include "slider/solver.h"
#include <queue>
#include <map>
#include <vector>
#include <cmath>
#include <set>
#include <algorithm>

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
  std::vector<Direction> path;
  int g = 0; 
  int h = 0; 

  bool operator>(const Node& other) const {
    return (g + h) > (other.g + other.h);
  }
};

} // namespace

Solution Solver::Solve(const BoardState& start_state) {
  if (start_state.IsSolved()) return {{}, true};

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
  std::map<BoardState, int> visited;

  open_set.push({start_state, {}, 0, GetHeuristic(start_state)});
  visited[start_state] = 0;

  int nodes_limit = 100000; 
  int nodes_checked = 0;

  while (!open_set.empty()) {
    Node current = open_set.top();
    open_set.pop();

    if (current.state.IsSolved()) {
      return {current.path, true};
    }

    if (nodes_checked++ > nodes_limit) break;

    Board board_current;
    board_current.SetState(current.state);
    auto valid_moves = board_current.GetValidMoves();

    for (Direction dir : valid_moves) {
      Board board_next = board_current;
      board_next.Move(dir);
      BoardState next_state = board_next.GetState();

      if (visited.find(next_state) == visited.end() || visited[next_state] > current.g + 1) {
        visited[next_state] = current.g + 1;
        std::vector<Direction> next_path = current.path;
        next_path.push_back(dir);
        open_set.push({next_state, next_path, current.g + 1, GetManhattanDistance(next_state)});
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
