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
#include <csignal>
namespace pong
{
  LocalServer::LocalServer(Volume v) noexcept : quadtree_(v, 3, 5)
  {
    this->loop_ = uv_loop_new();

    std::vector<char*> args;
    args.push_back((char*) "python");
    args.push_back((char*) "plugins/ppmlib.py");
    args.push_back(NULL);
    ipc::Spawn_Options opt;
    opt.args = &args[0];
    opt.cwd = NULL;
    this->install_plugin(make_json_plugin<Child_Process>(opt));

    this->log_.log(Severity::Info, "Initializing LocalServer");
  }
  LocalServer::~LocalServer() noexcept
  {
    this->log_.log(Severity::Info, "Uninitializing LocalServer");

    uv_run(this->loop_, UV_RUN_DEFAULT);
    uv_loop_delete(this->loop_);
  }
  // LocalServer function implementations.
  void LocalServer::set_destination(id_type id, math::vector<double> dest)
  {
    // Might throw an exception, fine let it throw!
    Object obj = this->quadtree_.find_object(id);
    PhysicsOptions& physobj = obj.physics_options;
    if(physobj.type != PhysicsType::Paddle)
    {
      physobj.type = PhysicsType::Paddle;
      physobj.paddle_options = PaddleOptions{};
    }
    physobj.paddle_options.destination = dest;

    this->quadtree_.set_object(id, obj);
  }
  void LocalServer::set_velocity(id_type id, math::vector<double> vel)
  {
    Object obj = this->quadtree_.find_object(id);
    PhysicsOptions& physopt = obj.physics_options;
    if(physopt.type != PhysicsType::Ball)
    {
      physopt.type = PhysicsType::Ball;
      physopt.ball_options = BallOptions{};
    }
    physopt.ball_options.velocity = vel;

    this->quadtree_.set_object(id, obj);
  }

  Object LocalServer::find_object(id_type id) const
  {
    return this->quadtree_.find_object(id);
  }
  std::vector<id_type> LocalServer::objects() const noexcept
  {
    return this->quadtree_.obj_manager().ids();
  }

  /*!
   * \brief Inserts an object into the Quadtree.
   *
   * \note If the object will intersect with any other object the new one *is
   * not* inserted.
   */
  id_type LocalServer::insert(const Object& o) noexcept
  {
    // Do this first so that if it fails we can avoid this *relatively*
    // hefty calculation.
    id_type id = this->quadtree_.insert(o);
    if(!id) return id;

    const Volume& v = o.volume;
    auto containing_nodes = find_containing_nodes(this->quadtree_.root(), v);
    for(const Quadtree::node_type* n : containing_nodes)
    {
      for(id_type col_id : n->get_data()->ids)
      {
        if(col_id == id) continue;
        Volume col_v = n->get_data()->objs->find_object(col_id).volume;

        if(intersecting(v, col_v))
        {
          this->quadtree_.erase(id);
          return 0;
        }
      }
    }
    return id;
  }

  math::vector<double> get_displacement(Object obj) noexcept
  {
    math::vector<double> disp;
    if(isPaddle(obj))
    {
      disp = obj.physics_options.paddle_options.destination - obj.volume.pos;
    }
    else
    {
      disp = obj.physics_options.ball_options.velocity;
    }

    // Dilute our displacement factoring in our movement constraints.
    return constrain(disp, obj.physics_options.constraints);
  }

  void reflect_ball(math::vector<double>& velocity, VolumeSides sides)
  {
    if(sides & (VolumeSide::Left | VolumeSide::Right))
    {
      velocity.x *= -1;
    }
    if(sides & (VolumeSide::Top | VolumeSide::Bottom))
    {
      velocity.y *= -1;
    }
  }

  void try_snap(Object& o1, Object& o2) noexcept
  {
    VolumeSides cs = closest_side(o2.volume, o1.volume);
    o1.volume.pos += outside_snap(o1.volume,
                                  cs & ~o1.physics_options.constraints,
                                  o2.volume);
    Volume bounds = {{0,0}, 1000, 1000};
    VolumeSides wall_sides = extending_sides(o1.volume, bounds);
    o1.volume.pos += inside_snap(o1.volume, wall_sides, bounds);

    cs = flip(cs);
    o2.volume.pos += outside_snap(o2.volume,
                                  cs & ~o2.physics_options.constraints,
                                  o1.volume);
    wall_sides = extending_sides(o2.volume, bounds);
    o2.volume.pos += inside_snap(o2.volume, wall_sides, bounds);
  }

  struct ModifiedObjectReference
  {
    id_type id;
    Object obj;
  };
  void LocalServer::react(ModifiedObjectReference& obj) noexcept
  {
    Volume bounds = {{0,0}, 1000, 1000};
    VolumeSides sides = extending_sides(obj.obj.volume, bounds);

    if(isBall(obj.obj))
    {
      // If we have a ball, change it's velocity accordingly. If necessary.
      reflect_ball(obj.obj.physics_options.ball_options.velocity, sides);
    }

    // Fire wall intersection signal if necessary.
    if(sides != VolumeSide::None)
    {
      this->obs_(sides, obj.id, this->quadtree_);
    }

    for(const auto& node : find_containing_nodes(this->quadtree_.root(),
                                                 obj.obj.volume))
    {
      for(id_type id : node->get_data()->ids)
      {
        if(id == obj.id) continue;

        Object& self = obj.obj;
        ModifiedObjectReference other_obj = {id,
                                             this->quadtree_.find_object(id)};
        Object& other = other_obj.obj;

        if(!intersecting(self.volume, other.volume)) continue;

        if(isBall(self))
        {
          if(isPaddle(other))
          {
            // Reflect off that paddle.
            VolumeSides cs = closest_side(self.volume, other.volume);
            reflect_ball(self.physics_options.ball_options.velocity, cs);
          }
          if(isBall(other))
          {
            // Swap velocity.
            std::swap(self.physics_options.ball_options.velocity,
                      other.physics_options.ball_options.velocity);
          }
        }

        if(isBall(self) || (isPaddle(self) && isPaddle(other)))
        {
          try_snap(self, other);
        }
        if(isPaddle(self) && isBall(other))
        {
          try_snap(other, self);
        }

        this->quadtree_.set_object(other_obj.id, other_obj.obj);
      }
    }
  }

  // Constraint utilities.
  void add_wall_constraints(Object& obj) noexcept
  {
    Volume bounds = {{0,0}, 1000, 1000};
    VolumeSides sides = extending_sides(obj.volume, bounds);

    // If we hit the wall *somewhere*, add a constraint.
    obj.physics_options.constraints |= sides;
  }
  void add_paddle_constraints(ModifiedObjectReference& obj,
                              Quadtree& q) noexcept
  {
    for(const auto& node : find_containing_nodes(q.root(), obj.obj.volume))
    {
      for(id_type other_id : node->get_data()->ids)
      {
        if(other_id == obj.id) continue;

        const Object& other_obj = q.find_object(other_id);
        if(!isPaddle(other_obj)) continue;

        if(intersecting(obj.obj.volume, other_obj.volume))
        {
          VolumeSides cs = closest_side(obj.obj.volume, other_obj.volume);
          obj.obj.physics_options.constraints |= cs;
        }
      }
    }
  }
  void add_ball_constraints(ModifiedObjectReference& obj,
                            Quadtree& q) noexcept
  {
    for(const auto& node : find_containing_nodes(q.root(), obj.obj.volume))
    {
      for(id_type other_id : node->get_data()->ids)
      {
        if(other_id == obj.id) continue;

        Object& self = obj.obj;

        ModifiedObjectReference other_obj = {other_id,q.find_object(other_id)};
        if(!isBall(other_obj.obj)) continue;
        const Object& other = other_obj.obj;

        if(!intersecting(self.volume, other.volume)) continue;

        VolumeSides cs = closest_side(self.volume, other.volume);
        self.physics_options.constraints |=
                                      other.physics_options.constraints & cs;
      }
    }
  }

  void generate_constraints(ModifiedObjectReference& obj, Quadtree& q) noexcept
  {
    obj.obj.physics_options.constraints = VolumeSide::None;

    // Add constraints from the wall.
    add_wall_constraints(obj.obj);

    // Add constraints from paddles.
    add_paddle_constraints(obj, q);

    // Add constraints from balls with other constraints.
    add_ball_constraints(obj, q);
  }

  void LocalServer::raytrace(id_type id) noexcept
  {
    ModifiedObjectReference obj = {id, this->quadtree_.find_object(id)};

    math::vector<double> diff = get_displacement(obj.obj);

    int steps = std::ceil(math::length(diff));
    double amount_simulated = 0.0;
    for(int i = 0; i < steps; ++i)
    {
      double to_sim = std::min(math::length(diff) - amount_simulated, 1.0);
      amount_simulated += to_sim;
      obj.obj.volume.pos += constrain(math::normalize(diff) * to_sim,
                                      obj.obj.physics_options.constraints);

      generate_constraints(obj, this->quadtree_);
    }

    // Respond to any collisions.
    react(obj);

    // Commit that object to the quad-tree.
    this->quadtree_.set_object(obj.id, obj.obj);
  }

  void LocalServer::step_() noexcept
  {
    uv_run(this->loop_, UV_RUN_NOWAIT);
    this->log_.step();
    {
      struct ModifyObject_Visitor : public boost::static_visitor<bool>
      {
        ModifyObject_Visitor(LocalServer& l, id_type id) : l_(l), id_(id) {}
        bool operator()(Object const& obj) const
        {
          try
          {
            l_.quadtree_.set_object(id_, obj);
          }
          catch(std::exception&)
          {
            return false;
          }
          return true;
        }
        bool operator()(Volume const& vol) const
        {
          try
          {
            Object obj = l_.quadtree_.find_object(id_);
            obj.volume = vol;
            l_.quadtree_.set_object(id_, obj);
          }
          catch(std::exception&)
          {
            return false;
          }
          return true;
        }
        bool operator()(PhysicsOptions const& phys) const
        {
          try
          {
            Object obj = l_.quadtree_.find_object(id_);
            obj.physics_options = phys;
            l_.quadtree_.set_object(id_, obj);
          }
          catch(std::exception&)
          {
            return false;
          }
          return true;
        }
      private:
        LocalServer& l_;
        id_type id_;
      };

      // Handle server requests
      struct RequestHandler : public boost::static_visitor<>
      {
        RequestHandler(LocalServer& l) : l_(l) {}
        void operator()(net::req::Null& req) noexcept {}
        void operator()(net::req::Log& req) noexcept
        {
          l_.log_.log(req.severity, req.msg);
          req.result.success = true;
        }
        void operator()(net::req::CreateObject& req) noexcept
        {
          req.result.obj_id = l_.quadtree_.insert(req.obj);
        }
        void operator()(net::req::DeleteObject& req) noexcept
        {
          l_.quadtree_.erase(req.obj_id);
          req.result.success = true;
        }
        void operator()(net::req::QueryObject& req) noexcept
        {
          try
          {
            req.result.obj = l_.find_object(req.obj_id);
            req.result.success = true;
          } catch(std::out_of_range& e)
          {
            req.result.success = false;
          }
        }
        void operator()(net::req::SetObject& req) noexcept
        {
          ModifyObject_Visitor visitor(l_, req.obj_id);
          req.result.success = boost::apply_visitor(visitor, req.data);
        }
      private:
        LocalServer& l_;
      };

      RequestHandler handler(*this);
      while(this->req_queue_.size() > 0)
      {
        auto pair = this->req_queue_.front();
        this->req_queue_.pop();
        boost::apply_visitor(handler, pair.first);
        if(pair.second) { pair.second(pair.first); }
      }
    }

    std::vector<id_type> ids = this->quadtree_.obj_manager().ids();
    using std::begin; using std::end;
    std::sort(begin(ids), end(ids), [&](id_type i1, id_type i2)
    {
      return isBall(this->quadtree_.obj_manager(), i1);
    });

    for(id_type id : ids)
    {
      raytrace(id);
    }
  }
}
