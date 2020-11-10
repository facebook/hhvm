(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external hh_fanout_dep_graph_stats_main : string -> unit
  = "hh_fanout_dep_graph_stats_main"

let go ~dep_graph =
  hh_fanout_dep_graph_stats_main dep_graph;
  Lwt.return_unit
