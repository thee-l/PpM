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
#include "LocalServer.h"
#include "exceptions.h"
#include "collision_util.h"
namespace pong
{
  PaddleID LocalServer::connect()
  {
    // We've done a loop, no more ids.
    if(++id_counter_ == 0x00) throw NoMoreClientsAvailable();
    // TODO Add support for configurable dimensions of the Paddle.
    this->world_.paddles.emplace_back(id_counter_,
                                      math::vector<int>(0, 0), 250, 40);
    return id_counter_;
  }
  Paddle& LocalServer::getPaddle(PaddleID id)
  {
    return findPaddleByID(this->world_, id);
  }
  const Paddle& LocalServer::getPaddle(PaddleID id) const
  {
    return findPaddleByID(this->world_, id);
  }
  Ball& LocalServer::getBall(BallID id)
  {
    return findBallByID(this->world_, id);
  }
  const Ball& LocalServer::getBall(BallID id) const
  {
    return findBallByID(this->world_, id);
  }

  void LocalServer::spawnBall(const Volume& vol, math::vector<int> vel)
  {
    if(++this->ball_id_counter_ == 0x00) throw NoMoreBallsAvailable();
    this->world_.balls.push_back(Ball{ball_id_counter_, vol, vel});
  }

  std::vector<PaddleID> LocalServer::paddles() const noexcept
  {
    const std::vector<Paddle>& paddles = this->world_.paddles;
    std::vector<PaddleID> ids(paddles.size());

    std::transform(paddles.begin(), paddles.end(), ids.begin(),
                   [&](const Paddle& paddle){ return paddle.id(); });
    return ids;
  }

  std::vector<BallID> LocalServer::balls() const noexcept
  {
    const std::vector<Ball>& balls = this->world_.balls;
    std::vector<BallID> ids(balls.size());

    std::transform(balls.begin(), balls.end(), ids.begin(),
                   [&](const Ball& ball){ return ball.id(); });
    return ids;
  }

  void LocalServer::step() noexcept
  {
    for(Paddle& paddle : this->world_.paddles)
    {
      math::vector<int>& pos = paddle.getPosition();
      math::vector<int>& next_pos = paddle.getNextPosition();
      math::vector<int> diff = {next_pos.x - pos.x, next_pos.y - pos.y};

      // We don't want to move to fast.
      // This method adds that terminal-velocity effect that is kind of
      // interesting.
      double new_length = std::min<double>(math::length<double>(diff), 5);

      std::vector<math::vector<int> > points =
                    raytrace(math::normalize<double>(diff) * new_length, pos);

      // The first point is 0,0. TODO add some sort of assertion of that.
      for(auto iter = points.begin() + 1; iter != points.end(); ++iter)
      {
        // Check to see if we *can* move.
        Volume vol = paddle.getVolume();
        vol.pos = *iter;
        bool can_move = true;

        // Will we intersect with another paddle.
        for(PaddleID other_id : this->paddles())
        {
          if(other_id == paddle.id()) continue;
          if(isIntersecting(vol, this->getPaddle(other_id).getVolume()))
          {
            can_move = false;
            break;
          }
        }

        // Will we intersect with the wall.
        if(isIntersectingWithWall(vol, 1000, 1000))
        {
          can_move = false;
        }

        if(can_move) // Nothing is obstructing us from moving to *iter.
        {
          // Handle the move. This is where we move the balls around.
          math::vector<int> new_diff = *iter - pos;
          for(BallID id : this->balls())
          {
            Ball& ball = this->getBall(id);
            if(isIntersecting(vol, ball.getVolume()))
            {
              // Which side of *the paddle.*
              VolumeSide side = findClosestSide(vol, ball.getVolume());
              switch(side)
              {
                case VolumeSide::Top:
                {
                  ball.getPosition().y = vol.pos.y - ball.diameter();
                  break;
                }
                case VolumeSide::Bottom:
                {
                  ball.getPosition().y = vol.pos.y + vol.height;
                  break;
                }
                case VolumeSide::Left:
                {
                  ball.getPosition().x = vol.pos.x - ball.diameter();
                  break;
                }
                case VolumeSide::Right:
                {
                  ball.getPosition().x = vol.pos.x + vol.width;
                  break;
                }
                default: break;
              }
            }
          }
          pos = *iter;
        }
      }
    }
  }
}
