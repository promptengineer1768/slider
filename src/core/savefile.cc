#include "slider/savefile.h"

#include <fstream>
#include <system_error>
#include <utility>

namespace slider {

SaveBoardStateResult SaveBoardStateToFile(const std::filesystem::path& path,
                                          const BoardState& state) {
  if (!state.IsValid()) {
    return {false, "Invalid board state"};
  }

  std::ofstream os(path, std::ios::binary);
  if (!os) {
    return {false, "Could not open file for writing"};
  }

  const std::string data = state.Serialize();
  os.write(data.data(), static_cast<std::streamsize>(data.size()));
  if (!os) {
    return {false, "Could not write file"};
  }

  return {true, {}};
}

LoadBoardStateResult LoadBoardStateFromFile(const std::filesystem::path& path,
                                            const SaveFileOptions& options) {
  std::error_code ec;
  const auto file_size = std::filesystem::file_size(path, ec);
  if (ec) {
    return {std::nullopt, "Could not read file size"};
  }

  if (static_cast<size_t>(file_size) > options.max_bytes) {
    return {std::nullopt, "Save file too large"};
  }

  std::ifstream is(path, std::ios::binary);
  if (!is) {
    return {std::nullopt, "Could not open file for reading"};
  }

  std::string content;
  content.resize(static_cast<size_t>(file_size));
  if (!content.empty()) {
    is.read(content.data(), static_cast<std::streamsize>(content.size()));
    if (!is) {
      return {std::nullopt, "Could not read file"};
    }
  }

  BoardState decoded = BoardState::Deserialize(content);
  if (!decoded.IsValid()) {
    return {std::nullopt, "Invalid save file"};
  }

  return {decoded, {}};
}

}  // namespace slider
