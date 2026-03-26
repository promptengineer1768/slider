#ifndef SLIDER_CORE_THEME_H_
#define SLIDER_CORE_THEME_H_

#include <string>

namespace slider {

struct ColorRGBA {
  int r = 0;
  int g = 0;
  int b = 0;
  int a = 255;
};

// Serializable theme definition loaded from resources/themes.json.
struct ThemeSpec {
  std::string name;
  ColorRGBA tile_color;
  ColorRGBA tile_highlight;
  ColorRGBA tile_shadow;
  ColorRGBA text_color;
  ColorRGBA bg_primary;
  ColorRGBA bg_secondary;
};

}  // namespace slider

#endif  // SLIDER_CORE_THEME_H_
