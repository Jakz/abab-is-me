#pragma once

#include "gfx/ViewManager.h"
#include "Common.h"

#include <memory>

class AssetCache;
struct MoveInfo;

namespace baba
{
  enum class D;
}

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
    void render();

    Scaler scaler;
    mutable point_t offset;
    size2d_t size;
    mutable coord_t tileSize;

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

    void movement(baba::D d);
    
    void renderLevelLink(coord_t x, coord_t y);

    float ratio() const;
    int32_t tick() const;

  public:
    GameView(ViewManager* director, Renderer* renderer);
    ~GameView();

    void handleKeyboardEvent(const events::KeyEvent& event);
    void handleMouseEvent(const events::MouseEvent& event);

    void levelLoaded();
  };

  class LevelSelectView : public View
  {
  public:
    using View::View;

    void render() override;
    void handleKeyboardEvent(const events::KeyEvent& event);
    void handleMouseEvent(const events::MouseEvent& event);
  };

  class MainMenuView : public View
  {
  public:
    using View::View;

    void render() override;
    void handleKeyboardEvent(const events::KeyEvent& event);
  };
}
