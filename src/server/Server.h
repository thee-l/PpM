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
#include <cstdint>
#include <vector>
#include "Object.h"
#include "ObjectManager.h"
#include "core/io/Logger.h"
#include "plugins.h"
#include "req.h"
namespace pong
{
  struct Server
  {
    Server() noexcept {};
    virtual ~Server() noexcept = default;

    /*!
     * \brief Sets the destination of the object making it a paddle if it isn't
     * already.
     *
     * \throws std::out_of_range if an object with that id doesn't exist.
     */
    virtual void set_destination(id_type id, math::vector<double> dest) = 0;
    /*!
     * \brief Sets the velocity of the object making it a ball if it isn't
     * already.
     *
     * \throw std::out_of_range if an object with that id doesn't exist.
     */
    virtual void set_velocity(id_type id, math::vector<double> vel) = 0;

    /*!
     * \brief Finds some object by id, returning a copy of it.
     *
     * \throws std::out_of_range if an object with that id doesn't exist.
     */
    virtual Object find_object(id_type id) const = 0;

    /*!
     * \brief Returns the id of every object in the world currently.
     */
    virtual std::vector<id_type> objects() const noexcept = 0;

    inline void step() noexcept;

    virtual Logger& logger() noexcept = 0;

    using request_callback = std::function<void (net::req::Request const&)>;
    inline void enqueue_request(net::req::Request const&,
                                request_callback) noexcept;

    inline void install_plugin(std::unique_ptr<Server_Plugin> sp) noexcept;
  protected:
    std::queue<std::pair<net::req::Request, request_callback> > req_queue_;
    std::vector<std::unique_ptr<Server_Plugin> > plugins_;

    virtual void step_() noexcept = 0;
  };

  inline void Server::enqueue_request(net::req::Request const& r,
                                      request_callback cb) noexcept
  {
    this->req_queue_.push({r, cb});
  }
  inline void Server::step() noexcept
  {
    for(auto& plugin : plugins_)
    {
      net::req::Request req;
      while(plugin->poll_request(req))
      {
        this->enqueue_request(req, [&](net::req::Request const& filled_req)
        {
          plugin->post_result(filled_req);
        });
      }
    }
    step_();
  }
  inline void
  Server::install_plugin(std::unique_ptr<Server_Plugin> sp) noexcept
  {
    plugins_.push_back(std::move(sp));
  }
}
