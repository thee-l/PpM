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
#include "collision_util.h"
namespace pong
{
  #define GENERATE_VOLUME_BOUNDS(vol) \
    int vol##_left = vol.pos.x, \
        vol##_right = vol.pos.x + vol.width - 1, \
        vol##_top = vol.pos.y, \
        vol##_bottom = vol.pos.y + vol.height - 1

  bool isIntersecting(const Volume& vol1, const Volume& vol2) noexcept
  {
    GENERATE_VOLUME_BOUNDS(vol1);
    GENERATE_VOLUME_BOUNDS(vol2);

    if((vol2_left <= vol1_left and vol1_left <= vol2_right) or
       (vol2_left <= vol1_right and vol1_right <= vol2_right) or
       (vol1_left <= vol2_left and vol2_right <= vol1_right))
    {
      if((vol2_top <= vol1_top and vol1_top <= vol2_bottom) or
         (vol2_top <= vol1_bottom and vol1_bottom <= vol2_bottom) or
         (vol1_top <= vol2_top and vol2_bottom <= vol1_bottom)) return true;
    }
    return false;
  }

  bool isIntersectingWithWall(const Volume& vol,
                              int width, int height) noexcept
  {
    if(vol.pos.x < 0 || vol.pos.y < 0) return true;

    if(vol.pos.x + vol.width - 1 > width - 1 ||
       vol.pos.y + vol.height - 1 > height - 1) return true;

    return false;
  }

  /*!
   * \brief Returns all discrete positions along a ray.
   */
  std::vector<math::vector<int> >
  raytrace(math::vector<double> ray) noexcept
  {
    math::vector<double> direction = math::normalize(ray);
    double length = math::length(ray);

    std::vector<math::vector<int> > points;

    for(double i = 0.0; i <= length; i += length / std::ceil(length))
    {
      math::vector<int> point = direction * i;
      if(std::find(points.begin(), points.end(), point) == points.end())
      {
        points.push_back(std::move(point));
      }
    }

    return points;
  }

  auto raytrace(math::vector<double> ray, math::vector<int> start)
                                                     -> decltype(raytrace(ray))
  {
    auto ray_points = raytrace(ray);

    using std::begin; using std::end;
    std::transform(begin(ray_points), end(ray_points), begin(ray_points),
                   [&](const math::vector<int>& point){return point + start;});
    return ray_points;
  }
}
