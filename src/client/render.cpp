/*
 * PpM - Pong Plus More - A pong clone full of surprises written with C++11.
 * Copyright (C) 2013  Luke San Antonio
 *
 * You can contact me (Luke San Antonio) at lukesanantonio@gmail.com!
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "render.h"
#include "widgets/Label.h"
namespace pong
{
  void render_debug(const Game& game, SDL_Renderer* renderer) noexcept
  {
    std::string stats = "FPS: " + std::to_string(game.fps);
    Label<std::string> label(stats, 20, {}, {0xff, 0xff, 0xff, 0xff},
                             game.settings.font_face.get(),
                             game.settings.font_rasterizer.get());

    label.position({1000-label.surface_width(), 1000-label.surface_height()});
    label.render(renderer);
  }

  void render(SDL_Renderer* renderer, const Volume& volume) noexcept
  {
    SDL_Rect rect;
    rect.x = volume.pos.x;
    rect.y = volume.pos.y;
    rect.w = volume.width;
    rect.h = volume.height;
    SDL_RenderFillRect(renderer, &rect);
  }

  void render_volume_sides(SDL_Renderer* renderer,
                           const Volume& v,
                           VolumeSides sides,
                           double percentage) noexcept
  {
    if(sides & VolumeSide::Left)
    {
      SDL_Rect r;
      r.x = v.pos.x;
      r.y = v.pos.y;
      r.w = v.width * percentage;
      r.h = v.height;
      SDL_RenderFillRect(renderer, &r);
    }
    if(sides & VolumeSide::Right)
    {
      SDL_Rect r;
      auto right = v.pos.x + v.width;
      r.x = right - v.width * percentage;
      r.y = v.pos.y;
      r.w = std::ceil(right - r.x);
      r.h = v.height;
      SDL_RenderFillRect(renderer, &r);
    }
    if(sides & VolumeSide::Top)
    {
      SDL_Rect r;
      r.x = v.pos.x;
      r.y = v.pos.y;
      r.w = v.width;
      r.h = v.height * percentage;
      SDL_RenderFillRect(renderer, &r);
    }
    if(sides & VolumeSide::Bottom)
    {
      SDL_Rect r;
      auto bottom = v.pos.y + v.height;
      r.x = v.pos.x;
      r.y = bottom - v.height * percentage;
      r.w = v.width;
      r.h = std::ceil(bottom - r.y);
      SDL_RenderFillRect(renderer, &r);
    }
  }

  void render_wireframe(SDL_Renderer* r, Volume const& vol) noexcept
  {
    GENERATE_VOLUME_BOUNDS(vol);

    SDL_RenderDrawLine(r, vol_left, vol_top, vol_right, vol_top);
    SDL_RenderDrawLine(r, vol_right, vol_top, vol_right, vol_bottom);
    SDL_RenderDrawLine(r, vol_right, vol_bottom, vol_left, vol_bottom);
    SDL_RenderDrawLine(r, vol_left, vol_bottom, vol_left, vol_top);
  }
}
