/*
   Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef DEPTH_FIRST_SEARCH_INCLUDED
#define DEPTH_FIRST_SEARCH_INCLUDED

#include <functional>
#include <set>
#include <stack>
#include <tuple>

/**
  @file include/depth_first_search.h
*/

/**
  Performs a Depth First Search algorithm on a graph defined by a set of
  vertexes being an subset of universum of values of TVertex type and, and set
  of directed edges generated on demand by get_neighbors() method supplied.
  Search starts with a selected vertex, for each vertex that is encountered a
  visitor_start() method supplied is called and when the DFS finishes traversal
  from that vertex, a visitor_end() method is called. The set of visited nodes
  is maintained, so the input graph don't have to be a Directed Acyclic Graph.

  @param start_vertex A vertex to start a search from.
  @param visitor_start A method or lambda that will be called when a specified
    vertex starts to be processed, the vertex is supplied as the only argument.
  @param visitor_end A method or lambda that will be called when a specified
    vertex ends to be processed, i.e. all its neighbors were already processed,
    the vertex is supplied as the only argument.
  @param get_neighbors A method or lambda that takes a vertex as an argument and
    returns a enumerable list of all vertexes to which the edges exists.
  @param [in,out] visited_set A set of vertexes that were visited. This can be
    used to run multiple DFS runs and assure any vertex will not be visited more
    than once.
*/
template <typename TVertex, typename TVisit_start, typename TVisit_end,
          typename TGet_neighbors, typename TVertex_less = std::less<TVertex>>
void depth_first_search(TVertex start_vertex, TVisit_start visitor_start,
                        TVisit_end visitor_end, TGet_neighbors get_neighbors,
                        std::set<TVertex> &visited_set = std::set<TVertex>{}) {
  /* A constant for a index denoting if the search from specified vertex has
    just been started of just ends. */
  constexpr int START_VISITING = 0;
  /* A constant for a index denoting the vertex. */
  constexpr int CURRENT_VERTEX = 1;

  /* An actual stack for search. */
  std::stack<std::tuple<bool, TVertex>> stack;
  /* Check if this vertex was not already visited - this may happen if
    the visited_set was returned by another DFS run. */
  if (!visited_set.insert(start_vertex).second) {
    return;
  }
  stack.emplace(true, start_vertex);

  while (!stack.empty()) {
    std::tuple<bool, TVertex> &elem = stack.top();
    if (std::get<START_VISITING>(elem)) {
      visitor_start(std::get<CURRENT_VERTEX>(elem));

      std::get<START_VISITING>(elem) = false;
      for (TVertex neighbor : get_neighbors(std::get<CURRENT_VERTEX>(elem))) {
        /* Check if this vertex was not visited yet in this or previous runs. */
        if (visited_set.insert(neighbor).second) {
          stack.emplace(true, neighbor);
        }
      }
    } else {
      visitor_end(std::get<CURRENT_VERTEX>(elem));
      stack.pop();
    }
  }
}

#endif
