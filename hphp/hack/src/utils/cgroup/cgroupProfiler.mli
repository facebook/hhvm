(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Profiling : sig
  type t

  val empty : t

  val record_stats :
    stage:string -> metric:string -> value:float -> profiling:t -> unit
end

val collect_cgroup_stats :
  profiling:Profiling.t -> stage:string -> (unit -> 'a) -> 'a

val profile_memory :
  event:[ `Init of string | `Recheck of string ] -> (Profiling.t -> 'a) -> 'a

val log_to_scuba : stage:string -> profiling:Profiling.t -> unit

val print_summary_memory_table : event:[ `Init | `Recheck of string ] -> unit
