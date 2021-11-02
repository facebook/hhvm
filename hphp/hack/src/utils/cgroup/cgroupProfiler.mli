(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Profiling : sig
  type t
end

type update_token

val update_cgroup_total : float -> update_token -> unit

val collect_cgroup_stats :
  profiling:Profiling.t -> stage:string -> (update_token -> 'a) -> 'a

val profile_memory :
  event:[ `Init of string | `Recheck of string ] -> (Profiling.t -> 'a) -> 'a

val print_summary_memory_table : event:[ `Init | `Recheck of string ] -> unit
