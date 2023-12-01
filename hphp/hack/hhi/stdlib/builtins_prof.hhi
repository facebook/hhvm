<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace {

  const int OBJPROF_FLAGS_DEFAULT;
  const int OBJPROF_FLAGS_USER_TYPES_ONLY;
  const int OBJPROF_FLAGS_PER_PROPERTY;

}

namespace HH {

  //////////////////////////////////////////////////////////////////
  // Objprof

  type ObjprofPathsStats = shape(
    'refs' => int,
  );

  type ObjprofObjectStats = shape(
    'instances' => int,
    'bytes' => int,
    'bytes_normalized' => float,
    'paths' => ?darray<string, ObjprofPathsStats>,
  );

  type ObjprofStringStats = shape(
    'dups' => int,
    'refs' => int,
    'srefs' => int,
    'path' => string,
  );

  function thread_memory_stats(
  ): darray<string, int>; // auto-imported from HH namespace

  function thread_mark_stack(): void; // auto-imported from HH namespace

  function objprof_get_data(
    int $flags = \OBJPROF_FLAGS_DEFAULT,
    varray<string> $exclude_list = vec[],
  ): darray<string, ObjprofObjectStats>; // auto-imported from HH namespace

  function objprof_get_paths(
    int $flags = \OBJPROF_FLAGS_DEFAULT,
    varray<string> $exclude_list = vec[],
  ): darray<string, ObjprofObjectStats>; // auto-imported from HH namespace

  //////////////////////////////////////////////////////////////////
  // Heap graph

  function heapgraph_create(): resource; // auto-imported from HH namespace
  function heapgraph_stats(
    resource $heapgraph,
  ): darray<string, int>; // auto-imported from HH namespace
  function heapgraph_foreach_node(
    resource $heapgraph,
    mixed $callback,
  ): void; // auto-imported from HH namespace
  function heapgraph_foreach_edge(
    resource $heapgraph,
    mixed $callback,
  ): void; // auto-imported from HH namespace
  function heapgraph_foreach_root(
    resource $heapgraph,
    mixed $callback,
  ): void; // auto-imported from HH namespace
  function heapgraph_dfs_nodes(
    resource $heapgraph,
    varray<int> $roots,
    varray<int> $skips,
    mixed $callback,
  ): void; // auto-imported from HH namespace
  function heapgraph_dfs_edges(
    resource $heapgraph,
    varray<int> $roots,
    varray<int> $skips,
    mixed $callback,
  ): void; // auto-imported from HH namespace
  function heapgraph_node(
    resource $heapgraph,
    int $index,
  ): darray<string, mixed>; // auto-imported from HH namespace
  function heapgraph_edge(
    resource $heapgraph,
    int $index,
  ): darray<string, mixed>; // auto-imported from HH namespace
  function heapgraph_node_in_edges(
    resource $heapgraph,
    int $index,
  ): varray<darray<string, mixed>>; // auto-imported from HH namespace
  function heapgraph_node_out_edges(
    resource $heapgraph,
    int $index,
  ): varray<darray<string, mixed>>; // auto-imported from HH namespace

  function set_mem_threshold_callback(int $threshold, mixed $callback): void;

}
