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
 * \file SinglePlayerGameState.h
 * \brief Contains declarations for the SinglePlayerGameState GameState.
 */
#pragma once
#include "GameState.h"
#include "Paddle.h"
#include "Ball.h"
#include "Game.h"

namespace pong
{
  class SinglePlayerGameState : public GameState
  {
  public:
    SinglePlayerGameState(Game&) noexcept;
    SinglePlayerGameState(const SinglePlayerGameState&) = delete;
    SinglePlayerGameState(SinglePlayerGameState&&) = delete;
    SinglePlayerGameState& operator=(const SinglePlayerGameState&) = delete;
    SinglePlayerGameState& operator=(SinglePlayerGameState&&) = delete;
    virtual ~SinglePlayerGameState() = default;

    virtual void handleEvent(const SDL_Event&) noexcept override;

    virtual void update() noexcept override {}

    Paddle topPaddle_;
    Paddle bottomPaddle_;

    Ball ball_;

    Game& game_;
  };
};
