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
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "core/common/vector.h"
#include "json/json.h"
#include "core/render/text.h"
#include "core/io/Logger.h"
namespace pong
{
  struct PluginInfo
  {
    std::string file;
    bool enabled;
  };
  struct ClientSettings
  {
    ClientSettings(std::string const& filename, Logger* l = nullptr) noexcept;
    ClientSettings(Logger* l = nullptr) noexcept;

    std::unique_ptr<text::Face> font_face;
    std::unique_ptr<text::Rasterizer> font_rasterizer;
    math::vector<int> extent;
    std::vector<PluginInfo> plugins;

    bool render_debug = false;
  };

  ClientSettings load_config(Logger* l) noexcept;
}
