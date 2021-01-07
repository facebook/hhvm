(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
external hh_fanout_dep_graph_is_subgraph_main : string -> string -> unit
  = "hh_fanout_dep_graph_is_subgraph_main"

let go ~sub ~super =
  hh_fanout_dep_graph_is_subgraph_main sub super;
  Lwt.return_unit
