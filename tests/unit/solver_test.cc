#include <gtest/gtest.h>
#include "slider/solver.h"
#include "slider/board.h"

namespace slider {

TEST(SolverTest, SolveSolved) {
  Board board(3);
  auto sol = Solver::Solve(board.GetState());
  EXPECT_TRUE(sol.success);
  EXPECT_EQ(sol.moves.size(), 0);
}

TEST(SolverTest, SolveShort) {
  Board board(3);
  // 1 2 3
  // 4 5 6
  // 7 0 8 (Move 8 right -> Up in empty)
  board.Move(Direction::kLeft); // Empty moves left
  
  auto sol = Solver::Solve(board.GetState());
  EXPECT_TRUE(sol.success);
  EXPECT_EQ(sol.moves.size(), 1);
  EXPECT_EQ(sol.moves[0], Direction::kRight);
}

TEST(SolverTest, SolveNSteps) {
  Board board(3);
  board.Move(Direction::kUp);
  board.Move(Direction::kLeft);
  
  auto sol = Solver::SolveNSteps(board.GetState(), 1);
  EXPECT_TRUE(sol.success);
  EXPECT_EQ(sol.moves.size(), 1);
}

} // namespace slider
