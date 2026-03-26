#ifndef SLIDER_CORE_SAVEFILE_H_
#define SLIDER_CORE_SAVEFILE_H_

#include "slider/state.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace slider {

struct SaveFileOptions {
  // Maximum save file size accepted by the loader.
  size_t max_bytes = 1024 * 10;  // 10KB
};

bool SaveBoardStateToFile(const std::filesystem::path& path,
                          const BoardState& state,
                          std::string* error = nullptr);

std::optional<BoardState> LoadBoardStateFromFile(const std::filesystem::path& path,
                                                 const SaveFileOptions& options,
                                                 std::string* error = nullptr);

}  // namespace slider

#endif  // SLIDER_CORE_SAVEFILE_H_
