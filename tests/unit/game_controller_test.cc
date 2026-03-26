#include <gtest/gtest.h>

#include "slider/game_controller.h"

#include <filesystem>

namespace slider {

TEST(GameControllerTest, LoadsThemesFromResources) {
  GameController controller(std::filesystem::current_path() / "bin");
  EXPECT_FALSE(controller.GetThemes().empty());
  EXPECT_TRUE(controller.SetThemeIndex(0));
  EXPECT_EQ(controller.GetThemeIndex(), 0);
}

TEST(GameControllerTest, ResolvesThemeAsset) {
  GameController controller(std::filesystem::current_path() / "bin");
  const auto path = controller.ResolveResourcePath("themes.json");
  EXPECT_FALSE(path.empty());
}

}  // namespace slider
