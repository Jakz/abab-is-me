#pragma once

#include "MainView.h"
#include "ViewManager.h"

using namespace baba;
using namespace ui;

void LevelSelectView::render()
{
  auto r = gvm->renderer();

  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
  SDL_RenderClear(r);
}

void LevelSelectView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE: gvm->exit(); break;
    }
  }
}

void LevelSelectView::handleMouseEvent(const SDL_Event& event)
{

}