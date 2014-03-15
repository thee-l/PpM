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
/*!
 * \file Label.cpp
 * \brief Contains definitions for the Label class.
 */
#include "Label.h"
#include "common/crash.hpp"
namespace pong
{
  Texture_Cache make_label_cache(Label& l)
  {
    Texture_Cache c;
    c.gen_func([](Texture_Cache::ptr_type p,
                  Surface_Cache& s, SDL_Renderer*& r)
    {
      if(p) return p;
      p.reset(SDL_CreateTextureFromSurface(r, s.cache()));
      return p;
    });

    c.grab_dependency<0>().gen_func(
    [&l](Surface_Cache::ptr_type p)
    {
      if(p) return p;
      p = std::move(l.font_renderer()->render_text(l.text(), l.text_height(),
                                                   l.text_color(),
                                                   l.back_color()));
      return p;
    });
    return c;
  }

  /*!
   * \brief Initializes a label with everything necessary to start rendering.
   *
   * \param text The text to display. Passed to pong::render_text.
   * \param text_height The height of the text in pixels. Passed to
   * pong::render_text.
   * \param pos The position of the top left corner where the surface will be
   * blitted.
   * \param font_renderer The font renderer implementation to use when
   * rasterizing the text!
   *
   * \note The text will be rendered monochrome in white. The background will
   * be solid black.
   */
  Label::Label(const std::string& text,
               int text_height,
               math::vector<int> pos,
               FontRenderer* font_renderer) noexcept :
               text_(text),
               text_height_(text_height),
               pos_(pos),
               font_renderer_(font_renderer),
               cache_(make_label_cache(*this))
  {
    //Set defaults.
    //Text color default: white
    this->text_color_.r = 0xff;
    this->text_color_.g = 0xff;
    this->text_color_.b = 0xff;

    //Background color default: black
    this->back_color_.r = 0x00;
    this->back_color_.g = 0x00;
    this->back_color_.b = 0x00;
  }

  /*!
   * \brief Initializes a label with everything + a custom text and background
   * color.
   *
   * \param text The text of the label.
   * \param text_height The pixel size of the text.
   * \param pos The position of the label when blitting.
   * \see Label::render(SDL_Surface*)
   * \param text_color The color of the text.
   * \param back_color The color of the empty space (if any) in the generated
   * SDL_Surface*.
   * \param font_renderer The existing FontRenderer implementation, will
   * not be deleted at any time!
   */
  Label::Label(const std::string& text,
               int text_height,
               math::vector<int> pos,
               SDL_Color text_color,
               SDL_Color back_color,
               FontRenderer* font_renderer) noexcept :
               text_(text),
               text_height_(text_height),
               pos_(pos),
               text_color_(text_color),
               back_color_(back_color),
               font_renderer_(font_renderer),
               cache_(make_label_cache(*this)) {}

  /*!
   * \brief Copy constructor.
   *
   * Does not copy the cache.
   */
  Label::Label(const Label& label) noexcept :
               text_(label.text_),
               text_height_(label.text_height_),
               pos_(label.pos_),
               text_color_(label.text_color_),
               back_color_(label.back_color_),
               font_renderer_(label.font_renderer_),
               cache_(label.cache_){}
  /*!
   * \brief Move constructor.
   *
   * Moves the cache from the object passed in, notably.
   */
  Label::Label(Label&& label) noexcept :
               text_(std::move(label.text_)),
               text_height_(label.text_height_),
               pos_(label.pos_),
               text_color_(label.text_color_),
               back_color_(label.back_color_),
               font_renderer_(label.font_renderer_),
               cache_(std::move(label.cache_)) {}

  /*!
   * \brief Copy assignment operator.
   *
   * Does not copy the cache, but invalidates it, just in case.
   */
  Label& Label::operator=(const Label& label) noexcept
  {
    this->text(label.text_);
    this->text_height(label.text_height_);

    this->position(label.pos_);

    this->text_color(label.text_color_);
    this->back_color(label.back_color_);

    this->cache_ = label.cache_;

    return *this;
  }

  /*!
   * \brief Move assignment operator.
   *
   * Moves the cache from the other object to `this` one, notably.
   */
  Label& Label::operator=(Label&& label) noexcept
  {
    this->text(std::move(label.text_));
    this->text_height(label.text_height_);

    this->position(label.pos_);

    this->text_color(label.text_color_);
    this->back_color(label.back_color_);

    this->cache_ = std::move(label.cache_);

    return *this;
  }

  /*!
   * \brief Renders the label using the passed in renderer.
   */
  void Label::render(SDL_Renderer* renderer) const
  {
    if(!renderer) return;

    this->cache_.set_dependency<1>(renderer);

    SDL_Rect dest;
    dest.x = this->pos_.x;
    dest.y = this->pos_.y;
    dest.w = this->getSurfaceWidth();
    dest.h = this->getSurfaceHeight();
    SDL_RenderCopy(renderer, this->cache_.cache(), NULL, &dest);
  }
  void Label::render(SDL_Surface* surface) const
  {
    SDL_Rect dest;
    dest.x = this->pos_.x;
    dest.y = this->pos_.y;
    dest.w = this->getSurfaceWidth();
    dest.h = this->getSurfaceHeight();
    SDL_BlitSurface(this->cache_.grab_dependency<0>().cache(),
                    NULL, surface, &dest);
  }
}
