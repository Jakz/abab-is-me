#include "ViewManager.h"

#include "gfx/views/Views.h"

#include "SdlHelper.h"
#include "gfx/Gfx.h"

using namespace ui;

ui::ViewManager::ViewManager() : _renderer(new Renderer(this))
{
  _renderer = new Renderer(this);
  
  _views.gameView = new GameView(this, _renderer);
  _views.levelSelectView = new LevelSelectView(this, _renderer);
  _views.mainMenu = new MainMenuView(this, _renderer);

  _view = _views.gameView;
}

bool ui::ViewManager::init()
{
  if (!_renderer->init())
    return false;

  _assets.init(_renderer, R"(E:\Games\Steam\SteamApps\common\Baba Is You)");

  if (!loadData())
    return false;
  
  return true;
}

void ui::ViewManager::deinit()
{
  _font.reset();
  _renderer->deinit();
}

void ui::ViewManager::loop()
{
  _renderer->loop();
  _renderer->deinit();
}

bool ui::ViewManager::loadData()
{
  _font.reset(_renderer->loadImage("font.png"));
  _font->enableAlphaBlending();

  return true;
}

size2d_t ui::ViewManager::windowSize() const
{
  return size2d_t(854, 480);
}

void ui::ViewManager::handle(const events::KeyEvent& event)
{
  _view->handleKeyboardEvent(event);
}

void ui::ViewManager::handle(const events::MouseEvent& event)
{
  _view->handleMouseEvent(event);
}


void ui::ViewManager::render()
{
  _view->render();
}

void ui::ViewManager::text(const Texture* texture, const std::string& text, int32_t x, int32_t y)
{
  rect_t dest = rect_t(x, y, 0, 0);
  
  for (size_t i = 0; i < text.length(); ++i)
  {
    rect_t src = texture->rect(text[i]);

    dest.w = src.w;
    dest.h = src.h;
   
    _renderer->blit(texture, color_t{ .r = 255, .g = 255, .b = 255, .a = 255 }, src, dest);
    dest.x += dest.w + 2;
  }
}

void ui::ViewManager::text(const std::string& text, int32_t x, int32_t y)
{
  constexpr float scale = 2.0;
  constexpr int32_t GLYPHS_PER_ROW = 32;

  for (size_t i = 0; i < text.length(); ++i)
  {
    rect_t src = { 6 * (text[i] % GLYPHS_PER_ROW), 9 * (text[i] / GLYPHS_PER_ROW), 5, 8 };
    rect_t dest = { x + 6 * i * scale, y, 5 * scale, 8 * scale };
    _renderer->blit(_font.get(), src, dest);
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
    _renderer->blit(_font.get(), src, dest);
  }

  SDL_SetTextureColorMod(_font->texture(), 255, 255, 255);
}
