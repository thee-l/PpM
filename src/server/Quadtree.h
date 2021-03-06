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
#include <memory>
#include <array>
#include <iterator>
#include <vector>
#include "core/common/utility.h"
#include "ObjectManager.h"
#include "Node.hpp"
namespace pong
{
  struct Node_Content
  {
    Node_Content(ObjectManager* const objs,
                 int max_objs,
                 int level,
                 int max_level,
                 const Volume& v = Volume(),
                 const std::vector<id_type>& ids = std::vector<id_type>())
                 : objs(objs), max_objs(max_objs), max_level(max_level),
                   current_level(level), v(v), ids(ids) {}
    ObjectManager *const objs;
    Volume v;
    int max_objs;
    int current_level;
    int max_level;
    std::vector<id_type> ids;
  };
  struct Quadtree
  {
    using node_type = Node<Node_Content>;

    Quadtree(const Volume& v, int max_objs = 5, int max_level = 5)
             : root_(std::make_unique<Node_Content>(&objs_, max_objs,
                                                    1, max_level,  v)) {}

    const Object& find_object(id_type) const;
    void set_object(id_type, const Object&);

    id_type insert(const Object& obj) noexcept;

    ObjectManager::iterator erase(ObjectManager::const_iterator pos);
    ObjectManager::iterator erase(ObjectManager::const_iterator pos,
                                  ObjectManager::const_iterator last);
    ObjectManager::size_type erase(id_type id);

    inline const node_type* root() const noexcept
    { return &this->root_; }
    inline const ObjectManager& obj_manager() const noexcept
    { return this->objs_; }

  private:
    ObjectManager objs_;
    node_type root_;
  };

  namespace detail
  {
    template <bool is_const, typename NT, class ND>
    using node_helper_t = std::conditional_t<is_const, const Node<NT, ND>,
                                                       Node<NT, ND> >;
    template <bool is_const, typename NT, class ND>
    std::vector<node_helper_t<is_const, NT, ND>* >
    find_containing_nodes(node_helper_t<is_const, NT, ND>* root,
                          const Volume& v) noexcept
    {
      if(!root) return {nullptr};
      if(!root->get_data()) return {nullptr};

      if(intersecting(root->get_data()->v, v))
      {
        // If leaf:
        if(root->children().empty()) return {root};

        // If parent:
        std::vector<node_helper_t<is_const, NT, ND>* > nodes;
        for(auto& child : root->children())
        {
          auto new_nodes = find_containing_nodes<is_const, NT, ND>(child, v);
          using std::begin; using std::end;
          nodes.insert(end(nodes), begin(new_nodes), end(new_nodes));
        }
        return nodes;
      }

      return {};
    }
  }

  template <typename NT, class ND>
  inline std::vector<Node<NT, ND>*>
  find_containing_nodes(Node<NT, ND>* root, const Volume& v) noexcept
  {
    return detail::find_containing_nodes<false, NT, ND>(root, v);
  }

  template <typename NT, class ND>
  inline std::vector<const Node<NT, ND>*>
  find_containing_nodes(const Node<NT, ND>* root, const Volume& v) noexcept
  {
    return detail::find_containing_nodes<true, NT, ND>(root, v);
  }

}
