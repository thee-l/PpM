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

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <string>
#include "json/json.h"

#include "../common/pif/helper.h"

namespace pong
{
  struct Invalid_Req_Parse {};
  struct Invalid_Id_Exception : Invalid_Req_Parse {};
  struct Invalid_Request_Exception : Invalid_Req_Parse {};

  struct null_t {};

  using req_id_t = boost::variant<null_t, int, std::string>;
  using optional_id_t = boost::optional<req_id_t>;

  struct Request
  {
    optional_id_t id;
    std::string method;
    boost::optional<Json::Value> params;
  };
}

BEGIN_FORMATTER_SCOPE
{
  DECLARE_FORMATTER(pong::null_t);
  DECLARE_FORMATTER(pong::req_id_t);
  DECLARE_FORMATTER(pong::Request);
}
END_FORMATTER_SCOPE