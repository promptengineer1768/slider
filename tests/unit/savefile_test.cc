#include <gtest/gtest.h>

#include "slider/savefile.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace slider {

TEST(SaveFileTest, RejectsOversizedFile) {
  const auto temp_path = std::filesystem::temp_directory_path() / "slider_oversized_save.sav";
  std::filesystem::remove(temp_path);

  {
    std::ofstream os(temp_path, std::ios::binary);
    ASSERT_TRUE(os.good());
    std::string content(11 * 1024, 'x');
    os.write(content.data(), static_cast<std::streamsize>(content.size()));
  }

  SaveFileOptions options;
  const auto loaded = LoadBoardStateFromFile(temp_path, options);

  EXPECT_FALSE(loaded.first.has_value());
  EXPECT_FALSE(loaded.second.empty());

  std::filesystem::remove(temp_path);
}

}  // namespace slider
