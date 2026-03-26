#ifndef SLIDER_CORE_SOLVER_H_
#define SLIDER_CORE_SOLVER_H_

#include "slider/state.h"
#include "slider/board.h"
#include <vector>
#include <map>

namespace slider {

struct Solution {
  std::vector<Direction> moves;
  bool success = false;
};

struct SolverOptions {
  // Hard cap on expanded nodes to prevent runaway memory/time usage.
  int nodes_limit = 100000;
};

class Solver {
 public:
  static Solution Solve(const BoardState& start_state);
  static Solution Solve(const BoardState& start_state, const SolverOptions& options);
  static Solution SolveNSteps(const BoardState& start_state, int n);
};

} // namespace slider

#endif // SLIDER_CORE_SOLVER_H_
