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
#include "Object.h"
namespace pong
{
  DEFINE_PROPERTY_VALUES(PaddleOptions);
  DEFINE_PROPERTY_VALUES(BallOptions);

  DEFINE_PARSER(PhysicsOptions, phys_json)
  {
    using vector_parser = parse::find_parser_t<math::vector<double> >;

    PhysicsOptions opt;
    if(phys_json["Velocity"].isObject())
    {
      opt.type = PhysicsType::Ball;
      opt.ball_options = BallOptions();

      opt.ball_options.velocity = vector_parser::parse(phys_json["Velocity"]);
    }
    else if(phys_json["Destination"].isObject())
    {
      opt.type = PhysicsType::Paddle;
      opt.paddle_options = PaddleOptions();
      opt.paddle_options.destination =
                                vector_parser::parse(phys_json["Destination"]);
    }
    return opt;
  }

  DEFINE_PROPERTY_VALUES(Object);
}
