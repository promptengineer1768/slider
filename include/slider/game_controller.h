#ifndef SLIDER_APP_GAME_CONTROLLER_H_
#define SLIDER_APP_GAME_CONTROLLER_H_

#include "slider/board.h"
#include "slider/savefile.h"
#include "slider/theme.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace slider {

// Owns the core game model used by the wxWidgets front-end.
class GameController {
 public:
  explicit GameController(std::filesystem::path resource_root = {});

  const Board& GetBoard() const { return board_; }
  Board& GetBoard() { return board_; }

  const std::vector<ThemeSpec>& GetThemes() const { return themes_; }
  int GetThemeIndex() const { return theme_index_; }
  bool SetThemeIndex(int index);

  int GetOptimalMoves() const { return optimal_moves_; }
  bool IsScrambling() const { return is_scrambling_; }
  void SetScrambling(bool scrambling) { is_scrambling_ = scrambling; }

  void NewGame();
  void ChangeSize(int size);

  bool Move(Direction dir);
  bool MoveTile(int tile_val);

  std::vector<Direction> Scramble(int max_moves);
  std::optional<std::vector<Direction>> Solve();
  std::optional<std::vector<Direction>> SolveNSteps(int n);

  SaveBoardStateResult SaveGame(const std::filesystem::path& path) const;
  LoadBoardStateResult LoadGame(const std::filesystem::path& path);

  std::filesystem::path ResolveResourcePath(const std::string& filename) const;

  void SetOptimalMoves(int moves) { optimal_moves_ = moves; }
  void ResetOptimalMoves() { optimal_moves_ = -1; }

 private:
  std::vector<ThemeSpec> LoadThemes() const;
  std::vector<ThemeSpec> GetDefaultThemes() const;

  std::filesystem::path resource_root_;
  Board board_;
  std::vector<ThemeSpec> themes_;
  int theme_index_ = 0;
  int optimal_moves_ = -1;
  bool is_scrambling_ = false;
};

}  // namespace slider

#endif  // SLIDER_APP_GAME_CONTROLLER_H_
