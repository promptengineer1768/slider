#include <gtest/gtest.h>

#include <wx/wx.h>
#include <wx/uiaction.h>
#include <wx/timer.h>

#include "slider/board.h"
#include "slider/state.h"

// -----------------------------------------------------------------------
// Headless logic tests: exercise the same move/state logic the GUI uses
// without needing a visible window or mouse simulation.
// -----------------------------------------------------------------------
namespace slider {

// Replicate the GUI's tile-click → direction → board-move pipeline.
static bool GUIStyleMove(Board& board, int tile_val) {
  auto dir = board.GetDirectionToMoveTile(tile_val);
  if (!dir) return false;
  return board.Move(*dir);
}

// -----------------------------------------------------------------------
// Solved state: 1 2 3 / 4 5 6 / 7 8 _   (3×3)
// Empty is at [2][2].  Only 6 (above) and 8 (left) are moveable.
// -----------------------------------------------------------------------
TEST(GUILogicTest, SolvedBoard_OnlyAdjacentTilesMoveable) {
  Board board(3);
  ASSERT_TRUE(board.IsSolved());

  // Tile 6 is directly above the empty → should move
  EXPECT_TRUE(board.GetDirectionToMoveTile(6).has_value());
  // Tile 8 is directly to the left of the empty → should move
  EXPECT_TRUE(board.GetDirectionToMoveTile(8).has_value());

  // Diagonals / non-adjacent → should NOT move
  EXPECT_FALSE(board.GetDirectionToMoveTile(1).has_value());
  EXPECT_FALSE(board.GetDirectionToMoveTile(3).has_value());
  EXPECT_FALSE(board.GetDirectionToMoveTile(5).has_value());
  EXPECT_FALSE(board.GetDirectionToMoveTile(9).has_value()); // doesn't exist
}

// After GUI-style-clicking tile 6, the board state must reflect the move.
TEST(GUILogicTest, ClickTile6_MovesIntoEmptySpace) {
  Board board(3);
  // Solved: 1 2 3 / 4 5 6 / 7 8 _
  int move_count_before = board.GetMoveCount();

  ASSERT_TRUE(GUIStyleMove(board, 6));

  // Expected: 1 2 3 / 4 5 _ / 7 8 6
  const auto& tiles = board.GetState().GetTiles();
  EXPECT_EQ(tiles[5], 0);  // empty now at row1 col2
  EXPECT_EQ(tiles[8], 6);  // 6 moved to row2 col2
  EXPECT_FALSE(board.IsSolved());
  EXPECT_EQ(board.GetMoveCount(), move_count_before + 1);
}

// After GUI-style-clicking tile 8, the board state must reflect the move.
TEST(GUILogicTest, ClickTile8_MovesIntoEmptySpace) {
  Board board(3);
  // Solved: 1 2 3 / 4 5 6 / 7 8 _
  ASSERT_TRUE(GUIStyleMove(board, 8));

  // Expected: 1 2 3 / 4 5 6 / 7 _ 8
  const auto& tiles = board.GetState().GetTiles();
  EXPECT_EQ(tiles[7], 0);   // empty now at row2 col1
  EXPECT_EQ(tiles[8], 8);   // 8 moved to row2 col2
  EXPECT_FALSE(board.IsSolved());
}

// Non-adjacent tile click must be a no-op.
TEST(GUILogicTest, ClickNonAdjacentTile_IsNoOp) {
  Board board(3);
  auto state_before = board.GetState();
  EXPECT_FALSE(GUIStyleMove(board, 1));
  EXPECT_EQ(board.GetState(), state_before);
  EXPECT_EQ(board.GetMoveCount(), 0);
}

// Sequence of moves then undo-style moves returns to solved.
TEST(GUILogicTest, MoveSequence_BackToSolved) {
  Board board(3);
  // Move 6 down, then 8 right, undo both
  ASSERT_TRUE(GUIStyleMove(board, 6));  // 6→empty
  ASSERT_TRUE(GUIStyleMove(board, 5));  // now 5 is beside new empty
  // Undo: move 5 back, then 6 back
  ASSERT_TRUE(GUIStyleMove(board, 5));
  ASSERT_TRUE(GUIStyleMove(board, 6));
  EXPECT_TRUE(board.IsSolved());
}

// -----------------------------------------------------------------------
// Verify GetDirectionToMoveTile returns the right direction so the
// animation offset will be consistent (tile moves TOWARD the empty).
// -----------------------------------------------------------------------
TEST(GUILogicTest, DirectionSemantics_TileAboveEmpty_ReturnsKUp) {
  Board board(3);
  // 6 is ABOVE the empty space.  Board::Move(kUp) moves empty UP, so tile slides down.
  auto dir = board.GetDirectionToMoveTile(6);
  ASSERT_TRUE(dir.has_value());
  EXPECT_EQ(*dir, Direction::kUp);
}

TEST(GUILogicTest, DirectionSemantics_TileLeftOfEmpty_ReturnsKLeft) {
  Board board(3);
  // 8 is to the LEFT of the empty.  Board::Move(kLeft) moves empty LEFT, tile slides right.
  auto dir = board.GetDirectionToMoveTile(8);
  ASSERT_TRUE(dir.has_value());
  EXPECT_EQ(*dir, Direction::kLeft);
}

// -----------------------------------------------------------------------
// 4×4 (classic 15-puzzle)
// Solved: 1..14 15 _   Empty at [3][3].
// -----------------------------------------------------------------------
TEST(GUILogicTest, FifteenPuzzle_OnlyAdjacentMoveable) {
  Board board(4);
  EXPECT_TRUE(board.GetDirectionToMoveTile(15).has_value()); // left of empty
  EXPECT_TRUE(board.GetDirectionToMoveTile(12).has_value()); // above empty
  EXPECT_FALSE(board.GetDirectionToMoveTile(1).has_value());
  EXPECT_FALSE(board.GetDirectionToMoveTile(11).has_value());
}

// -----------------------------------------------------------------------
// 5×5 variation
// -----------------------------------------------------------------------
TEST(GUILogicTest, FiveByFive_OnlyAdjacentMoveable) {
  Board board(5);
  // Solved: 1..24 _   Empty at [4][4].
  EXPECT_TRUE(board.GetDirectionToMoveTile(24).has_value()); // left of empty
  EXPECT_TRUE(board.GetDirectionToMoveTile(20).has_value()); // above empty
  EXPECT_FALSE(board.GetDirectionToMoveTile(1).has_value());
}

}  // namespace slider
