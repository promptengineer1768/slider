#include <gtest/gtest.h>

#include "slider/board.h"
#include "slider/state.h"

namespace slider {

TEST(StateValidityTest, Solvable4x4_SolvedIsValid) {
  BoardState solved(4, {1,  2,  3,  4,
                       5,  6,  7,  8,
                       9,  10, 11, 12,
                       13, 14, 15, 0});
  EXPECT_TRUE(solved.IsValid());
  EXPECT_TRUE(solved.IsSolved());
}

TEST(StateValidityTest, Solvable4x4_OneMoveFromSolvedIsValid) {
  Board b(4);
  ASSERT_TRUE(b.IsSolved());
  ASSERT_TRUE(b.Move(Direction::kLeft)); // move empty left
  EXPECT_TRUE(b.GetState().IsValid());
}

TEST(StateValidityTest, Unsolvable4x4_SwappedTilesIsInvalid) {
  // Swap 1 and 2 in a solved 4x4 (a known unsolvable configuration).
  BoardState unsolvable(4, {2,  1,  3,  4,
                           5,  6,  7,  8,
                           9,  10, 11, 12,
                           13, 14, 15, 0});
  EXPECT_FALSE(unsolvable.IsValid());
}

TEST(StateValidityTest, BoardSizeZeroDoesNotCrash) {
  Board b(0);
  EXPECT_FALSE(b.Move(Direction::kUp));
  EXPECT_TRUE(b.GetValidMoves().empty());
}

} // namespace slider

