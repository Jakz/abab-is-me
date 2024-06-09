#pragma once

#include "Common.h"
#include "SdlHelper.h"

#include "gfx/AssetCache.h"

#include <memory>

using Renderer = SDL;


namespace ui
{
  class ViewManager;

  class View
  {
  protected:
    ViewManager* _director;
    Renderer* _renderer;
    
  public:
    View(ViewManager* director, Renderer* renderer) : _director(director), _renderer(renderer) { }

    virtual void render() = 0;
    virtual void handleKeyboardEvent(const events::KeyEvent& event) { };
    virtual void handleMouseEvent(const events::MouseEvent& event) { };

    AssetCache* assets();
  };

  enum TextAlign
  {
    LEFT, CENTER, RIGHT
  };

  class GameView;
  class LevelSelectView;
  class MainMenuView;

  class ViewManager : public Director
  {
  public:
    using view_t = View;

    std::unique_ptr<Texture> _font;

  private:
    Renderer* _renderer;

    struct
    {
      GameView* gameView;
      LevelSelectView* levelSelectView;
      MainMenuView* mainMenu;
      
    } _views;

    view_t* _view;

    AssetCache _assets;

  public:
    ViewManager();

    bool loadData();

    void handle(const events::KeyEvent& event) override;
    void handle(const events::MouseEvent& event) override;
    void render() override;

    bool init();
    void deinit();

    void loop();

    virtual size2d_t windowSize() const;
    AssetCache* assets() { return &_assets; }

    Texture* font() const { return _font.get(); }

    //TODO: hacky cast to avoid header inclusion
    GameView* gameView() const { return _views.gameView; }

    int32_t textWidth(const std::string& text, float scale = 2.0f) const { return text.length() * scale * 4; }
    void text(const std::string& text, int32_t x, int32_t y, SDL_Color color, TextAlign align, float scale = 2.0f);
    void text(const std::string& text, int32_t x, int32_t y);

    void text(const Texture* font, const std::string& text, int32_t x, int32_t y);
  };

  inline AssetCache* View::assets() { return _director->assets(); }
}

