#include <gtest/gtest.h>

#include "slider/state.h"

namespace slider {

TEST(SerializationTest, RoundTripSolved3x3) {
  BoardState s(3, {1, 2, 3,
                   4, 5, 6,
                   7, 8, 0});

  const std::string encoded = s.Serialize();
  BoardState decoded = BoardState::Deserialize(encoded);

  EXPECT_TRUE(decoded.IsValid());
  EXPECT_EQ(decoded.GetSize(), 3);
  EXPECT_EQ(decoded.GetTiles(), s.GetTiles());
}

TEST(SerializationTest, RejectsTruncatedInput) {
  // Size says 3x3, but we only provide a few tiles.
  BoardState decoded = BoardState::Deserialize("3 1 2 3");
  EXPECT_FALSE(decoded.IsValid());
}

TEST(SerializationTest, RejectsNegativeSize) {
  BoardState decoded = BoardState::Deserialize("-1 0");
  EXPECT_FALSE(decoded.IsValid());
}

TEST(SerializationTest, RejectsInvalidTileSet) {
  // Duplicate tiles / missing 0.
  BoardState decoded = BoardState::Deserialize("3 1 1 2 3 4 5 6 7 8");
  EXPECT_FALSE(decoded.IsValid());
}

} // namespace slider

