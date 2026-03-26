#include "slider/game_controller.h"

#include "slider/scrambler.h"
#include "slider/solver.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string_view>
#include <utility>

namespace slider {
namespace {

std::string Trim(std::string_view input) {
  const auto first = input.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) return {};
  const auto last = input.find_last_not_of(" \t\r\n");
  return std::string(input.substr(first, last - first + 1));
}

bool ParseColorArray(std::string_view text, ColorRGBA* out) {
  if (!out) return false;
  const auto open = text.find('[');
  const auto close = text.find(']', open == std::string_view::npos ? 0 : open + 1);
  if (open == std::string_view::npos || close == std::string_view::npos || close <= open) {
    return false;
  }

  std::stringstream ss{std::string(text.substr(open + 1, close - open - 1))};
  char comma = 0;
  int r = 0;
  int g = 0;
  int b = 0;
  if (!(ss >> r)) return false;
  if (!(ss >> comma) || comma != ',') return false;
  if (!(ss >> g)) return false;
  if (!(ss >> comma) || comma != ',') return false;
  if (!(ss >> b)) return false;
  int a = 255;
  if (ss >> comma) {
    if (comma != ',') return false;
    if (!(ss >> a)) return false;
  }

  out->r = r;
  out->g = g;
  out->b = b;
  out->a = a;
  return true;
}

std::optional<std::string> ExtractStringField(std::string_view object, std::string_view key) {
  const std::string needle = "\"" + std::string(key) + "\"";
  const auto key_pos = object.find(needle);
  if (key_pos == std::string_view::npos) return std::nullopt;
  const auto colon = object.find(':', key_pos + needle.size());
  if (colon == std::string_view::npos) return std::nullopt;
  const auto first_quote = object.find('"', colon + 1);
  if (first_quote == std::string_view::npos) return std::nullopt;
  const auto second_quote = object.find('"', first_quote + 1);
  if (second_quote == std::string_view::npos) return std::nullopt;
  return std::string(object.substr(first_quote + 1, second_quote - first_quote - 1));
}

std::optional<ColorRGBA> ExtractColorField(std::string_view object, std::string_view key) {
  const std::string needle = "\"" + std::string(key) + "\"";
  const auto key_pos = object.find(needle);
  if (key_pos == std::string_view::npos) return std::nullopt;
  const auto colon = object.find(':', key_pos + needle.size());
  if (colon == std::string_view::npos) return std::nullopt;
  ColorRGBA color;
  if (!ParseColorArray(object.substr(colon + 1), &color)) return std::nullopt;
  return color;
}

std::optional<ThemeSpec> ParseThemeObject(std::string_view object) {
  ThemeSpec theme;
  const auto name = ExtractStringField(object, "name");
  const auto tile_color = ExtractColorField(object, "tile_color");
  const auto tile_highlight = ExtractColorField(object, "tile_highlight");
  const auto tile_shadow = ExtractColorField(object, "tile_shadow");
  const auto text_color = ExtractColorField(object, "text_color");
  const auto bg_primary = ExtractColorField(object, "bg_primary");
  const auto bg_secondary = ExtractColorField(object, "bg_secondary");
  if (!name || !tile_color || !tile_highlight || !tile_shadow || !text_color || !bg_primary || !bg_secondary) {
    return std::nullopt;
  }

  theme.name = *name;
  theme.tile_color = *tile_color;
  theme.tile_highlight = *tile_highlight;
  theme.tile_shadow = *tile_shadow;
  theme.text_color = *text_color;
  theme.bg_primary = *bg_primary;
  theme.bg_secondary = *bg_secondary;
  return theme;
}

std::optional<std::vector<ThemeSpec>> ParseThemesJson(const std::string& json) {
  std::vector<ThemeSpec> themes;
  const auto themes_key = json.find("\"themes\"");
  if (themes_key == std::string::npos) return std::nullopt;
  const auto array_open = json.find('[', themes_key);
  if (array_open == std::string::npos) return std::nullopt;

  size_t i = array_open + 1;
  bool saw_any_theme = false;
  while (i < json.size()) {
    while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i]))) {
      ++i;
    }
    if (i >= json.size() || json[i] == ']') break;
    if (json[i] != '{') {
      return std::nullopt;
    }

    int depth = 0;
    const size_t start = i;
    do {
      if (json[i] == '{') {
        ++depth;
      } else if (json[i] == '}') {
        --depth;
      }
      ++i;
    } while (i < json.size() && depth > 0);

    if (depth == 0) {
      auto theme = ParseThemeObject(std::string_view(json).substr(start, i - start));
      if (!theme) return std::nullopt;
      themes.push_back(*theme);
      saw_any_theme = true;
    } else {
      return std::nullopt;
    }
  }

  if (!saw_any_theme || themes.size() != 5) {
    return std::nullopt;
  }
  return themes;
}

ThemeSpec MakeTheme(const char* name,
                    ColorRGBA tile_color,
                    ColorRGBA tile_highlight,
                    ColorRGBA tile_shadow,
                    ColorRGBA text_color,
                    ColorRGBA bg_primary,
                    ColorRGBA bg_secondary) {
  return ThemeSpec{name, tile_color, tile_highlight, tile_shadow, text_color, bg_primary, bg_secondary};
}

}  // namespace

GameController::GameController(std::filesystem::path resource_root)
    : resource_root_(std::move(resource_root)),
      board_(3),
      themes_(LoadThemes()) {}

bool GameController::SetThemeIndex(int index) {
  if (index < 0 || index >= static_cast<int>(themes_.size())) return false;
  theme_index_ = index;
  return true;
}

void GameController::NewGame() {
  board_ = Board(board_.GetSize() > 0 ? board_.GetSize() : 3);
  optimal_moves_ = -1;
  is_scrambling_ = false;
}

void GameController::ChangeSize(int size) {
  board_ = Board(size);
  optimal_moves_ = -1;
  is_scrambling_ = false;
}

bool GameController::Move(Direction dir) {
  return board_.Move(dir);
}

bool GameController::MoveTile(int tile_val) {
  return board_.MoveTile(tile_val);
}

std::vector<Direction> GameController::Scramble(int max_moves) {
  const auto moves = Scrambler::Scramble(board_, max_moves);
  is_scrambling_ = true;
  optimal_moves_ = -1;
  return moves;
}

std::optional<std::vector<Direction>> GameController::Solve() {
  auto sol = Solver::Solve(board_.GetState());
  if (!sol.success) return std::nullopt;
  return sol.moves;
}

std::optional<std::vector<Direction>> GameController::SolveNSteps(int n) {
  auto sol = Solver::SolveNSteps(board_.GetState(), n);
  if (!sol.success) return std::nullopt;
  return sol.moves;
}

SaveBoardStateResult GameController::SaveGame(const std::filesystem::path& path) const {
  return SaveBoardStateToFile(path, board_.GetState());
}

LoadBoardStateResult GameController::LoadGame(const std::filesystem::path& path) {
  const auto result = LoadBoardStateFromFile(path, SaveFileOptions{});
  if (result.first) {
    board_.SetState(*result.first);
    optimal_moves_ = -1;
    is_scrambling_ = false;
    return result;
  }
  return result;
}

std::filesystem::path GameController::ResolveResourcePath(const std::string& filename) const {
  if (resource_root_.empty()) {
    return std::filesystem::path(filename);
  }

  const std::filesystem::path direct = resource_root_ / filename;
  if (std::filesystem::exists(direct)) return direct;

  const std::filesystem::path nested = resource_root_ / "resources" / filename;
  if (std::filesystem::exists(nested)) return nested;

  const std::filesystem::path cwd_direct = std::filesystem::current_path() / filename;
  if (std::filesystem::exists(cwd_direct)) return cwd_direct;

  const std::filesystem::path cwd_nested = std::filesystem::current_path() / "resources" / filename;
  if (std::filesystem::exists(cwd_nested)) return cwd_nested;

  return {};
}

std::vector<ThemeSpec> GameController::GetDefaultThemes() const {
  return {
      MakeTheme("Ocean",
                {0, 102, 204, 255}, {51, 153, 255, 255}, {0, 51, 153, 255}, {255, 255, 255, 255},
                {240, 248, 255, 255}, {176, 196, 222, 255}),
      MakeTheme("Forest",
                {34, 139, 34, 255}, {50, 205, 50, 255}, {0, 100, 0, 255}, {255, 255, 255, 255},
                {245, 255, 250, 255}, {143, 188, 143, 255}),
      MakeTheme("Lava",
                {255, 69, 0, 255}, {255, 140, 0, 255}, {139, 0, 0, 255}, {255, 255, 255, 255},
                {255, 245, 238, 255}, {255, 218, 185, 255}),
      MakeTheme("Modern",
                {60, 60, 60, 255}, {100, 100, 100, 255}, {30, 30, 30, 255}, {255, 255, 255, 255},
                {240, 240, 240, 255}, {200, 200, 200, 255}),
      MakeTheme("Cyber",
                {128, 0, 128, 255}, {255, 0, 255, 255}, {75, 0, 130, 255}, {0, 255, 255, 255},
                {10, 10, 10, 255}, {40, 40, 40, 255}),
  };
}

std::vector<ThemeSpec> GameController::LoadThemes() const {
  const auto themes_path = ResolveResourcePath("themes.json");
  if (themes_path.empty()) {
    return GetDefaultThemes();
  }

  std::ifstream is(themes_path);
  if (!is) {
    return GetDefaultThemes();
  }

  std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
  auto parsed = ParseThemesJson(json);
  if (!parsed) {
    return GetDefaultThemes();
  }
  return *parsed;
}

}  // namespace slider
