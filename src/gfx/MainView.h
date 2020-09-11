#pragma once

#include "ViewManager.h"
#include "Common.h"

class LevelRenderer;

namespace ui
{
  enum class Scaler
  {
    SCALE_TO_FIT,
    KEEP_AT_MOST_NATIVE
  };

  
  class GameView : public View
  {
  private:
    std::unique_ptr<LevelRenderer> levelRenderer;
    ViewManager* gvm;

    void render();

    Scaler scaler;
    point_t offset;
    size2d_t size;
    coord_t tileSize;

    struct
    {
      SDL_Surface* palette;
      SDL_Color outside;
      SDL_Color inside;
    } colors;

    point_t moveBounds[2];

#if MOUSE_ENABLED
    std::string hoverInfo;
#endif

    void updateMoveBounds();

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
