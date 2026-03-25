#include <gtest/gtest.h>
#include "slider/scrambler.h"
#include "slider/board.h"

namespace slider {

TEST(ScramblerTest, Scramble) {
  Board board(3);
  auto moves = Scrambler::Scramble(board, 20);
  
  EXPECT_FALSE(board.IsSolved());
  EXPECT_TRUE(board.GetState().IsValid());
}

} // namespace slider
