#pragma once

#include "SdlHelper.h"
#include "Common.h"

#include <memory>

namespace ui
{
  class View
  {
  public:
    virtual void render() = 0;
    virtual void handleKeyboardEvent(const SDL_Event& event) = 0;
    virtual void handleMouseEvent(const SDL_Event& event) = 0;
  };

  enum TextAlign
  {
    LEFT, CENTER, RIGHT
  };

  class GameView;
  class LevelSelectView;

  class ViewManager : public SDL<ViewManager, ViewManager>
  {
  public:
    using view_t = View;

    std::unique_ptr<Texture> _font;

  private:
    LevelSelectView* _levelSelectView;
    GameView* _gameView;
    view_t* _view;

  public:
    ViewManager();

    bool loadData();

    void handleKeyboardEvent(const SDL_Event& event, bool press);
    void handleMouseEvent(const SDL_Event& event);
    void render();

    void deinit();

    Texture* font() { return _font.get(); }

    //TODO: hacky cast to avoid header inclusion
    GameView* gameView() { return _gameView; }

    int32_t textWidth(const std::string& text, float scale = 2.0f) const { return text.length() * scale * 4; }
    void text(const std::string& text, int32_t x, int32_t y, SDL_Color color, TextAlign align, float scale = 2.0f);
    void text(const std::string& text, int32_t x, int32_t y);
  };
}

