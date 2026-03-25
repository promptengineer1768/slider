#include <gtest/gtest.h>
#include "slider/board.h"
#include <algorithm>

namespace slider {

TEST(BoardTest, Initialization) {
  Board board(3);
  EXPECT_EQ(board.GetSize(), 3);
  EXPECT_TRUE(board.IsSolved());
  EXPECT_EQ(board.GetMoveCount(), 0);
}

TEST(BoardTest, Moves) {
  Board board(3);
  // Solved state: 1 2 3 / 4 5 6 / 7 8 0
  // Empty is at (2, 2)
  
  // Move 6 down (empty moves up)
  EXPECT_TRUE(board.Move(Direction::kUp));
  EXPECT_FALSE(board.IsSolved());
  EXPECT_EQ(board.GetMoveCount(), 1);
  
  // Move 6 back up (empty moves down)
  EXPECT_TRUE(board.Move(Direction::kDown));
  EXPECT_TRUE(board.IsSolved());
}

TEST(BoardTest, MoveTile) {
  Board board(3);
  // 1 2 3
  // 4 5 6
  // 7 8 0
  
  // Move 8 to empty
  EXPECT_TRUE(board.MoveTile(8));
  EXPECT_EQ(board.GetState().GetTiles().back(), 8);
  EXPECT_FALSE(board.IsSolved());
}

} // namespace slider
