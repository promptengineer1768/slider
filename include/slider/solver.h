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

class Solver {
 public:
  static Solution Solve(const BoardState& start_state);
  static Solution SolveNSteps(const BoardState& start_state, int n);
};

} // namespace slider

#endif // SLIDER_CORE_SOLVER_H_
