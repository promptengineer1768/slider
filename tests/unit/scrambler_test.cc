#include <gtest/gtest.h>
#include "slider/scrambler.h"
#include "slider/board.h"

namespace slider {

TEST(ScramblerTest, Scramble) {
  Board scrambled(3);
  auto moves = Scrambler::Scramble(scrambled, 20);

  EXPECT_LE(moves.size(), 20u);
  EXPECT_TRUE(scrambled.GetState().IsValid());
  EXPECT_FALSE(scrambled.IsSolved());

  Board replay(3);
  for (Direction move : moves) {
    EXPECT_TRUE(replay.Move(move));
  }
  EXPECT_EQ(replay.GetState(), scrambled.GetState());
}

} // namespace slider
