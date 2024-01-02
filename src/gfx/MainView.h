#pragma once

#include "ViewManager.h"
#include "Common.h"

#include <memory>

class AssetCache;

namespace ui
{
  enum class Scaler
  {
    SCALE_TO_FIT,
    SCALE_TO_ATMOST_NATIVE,
    NATIVE_WITH_SCROLL
  };

  
  class GameView : public View
  {
  private:
    ViewManager* gvm;

    void render();

    Scaler scaler;
    point_t offset;
    size2d_t size;
    coord_t tileSize;

    struct
    {
      const Palette* palette;
      color_t outside;
      color_t inside;
      color_t grid;
    } colors;

    point_t moveBounds[2];

#if MOUSE_ENABLED
    std::string hoverInfo;
#endif

    void updateMoveBounds();

    void drawGrid(point_t b, size2d_t size, size2d_t count);

  public:
    GameView(ViewManager* gvm);
    ~GameView();

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);

    void levelLoaded();
  };

  class LevelSelectView : public View
  {
  private:
    ViewManager* gvm;

  public:
    LevelSelectView(ViewManager* gvm) : gvm(gvm) { }

    void render() override;
    void handleKeyboardEvent(const SDL_Event& event) override;
    void handleMouseEvent(const SDL_Event& event) override;
  };
}
