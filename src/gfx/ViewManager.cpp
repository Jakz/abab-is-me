#include "ViewManager.h"

#include "gfx/views/Views.h"

#include "gfx/Gfx.h"

using namespace ui;

ui::ViewManager::ViewManager() : SDL<ui::ViewManager, ui::ViewManager>(*this, *this),
_gameView(new GameView(this)), _levelSelectView(new LevelSelectView(this))
{
  _view = _gameView;

  _views._mainMenu = new MainMenuView(this);
}

void ui::ViewManager::deinit()
{
  _font.reset();
  SDL::deinit();
}

bool ui::ViewManager::loadData()
{
  SDL_Surface* font = IMG_Load("font.png");
  assert(font);

  auto fontTexture = SDL_CreateTextureFromSurface(_renderer, font);
  SDL_FreeSurface(font);

  _font.reset(new Texture(fontTexture, { font->w, font->h }, { }));
  SDL_SetTextureBlendMode(_font->texture(), SDL_BLENDMODE_BLEND);

  return true;
}

void ui::ViewManager::handleKeyboardEvent(const events::KeyEvent& event)
{
  _view->handleKeyboardEvent(event);
}

void ui::ViewManager::handleMouseEvent(const events::MouseEvent& event)
{
  _view->handleMouseEvent(event);
}


void ui::ViewManager::render()
{
  _view->render();
}

void ui::ViewManager::text(const std::string& text, int32_t x, int32_t y)
{
  constexpr float scale = 2.0;
  constexpr int32_t GLYPHS_PER_ROW = 32;

  for (size_t i = 0; i < text.length(); ++i)
  {
    rect_t src = { 6 * (text[i] % GLYPHS_PER_ROW), 9 * (text[i] / GLYPHS_PER_ROW), 5, 8 };
    rect_t dest = { x + 6 * i * scale, y, 5 * scale, 8 * scale };
    blit(_font.get(), src, dest);
  }
}

void ViewManager::text(const std::string& text, int32_t x, int32_t y, SDL_Color color, TextAlign align, float scale)
{
  constexpr int32_t GLYPHS_PER_ROW = 32;

  const int32_t width = text.size() * 6 * scale;

  if (align == TextAlign::CENTER)
    x -= width / 2;
  else if (align == TextAlign::RIGHT)
    x -= width;

  SDL_SetTextureColorMod(_font->texture(), color.r, color.g, color.b);

  for (size_t i = 0; i < text.length(); ++i)
  {
    rect_t src = { 6 * (text[i] % GLYPHS_PER_ROW), 9 * (text[i] / GLYPHS_PER_ROW), 5, 8 };
    rect_t dest = { x + 6 * i * scale, y, 5 * scale, 8 * scale };
    blit(_font.get(), src, dest);
  }

  SDL_SetTextureColorMod(_font->texture(), 255, 255, 255);
}