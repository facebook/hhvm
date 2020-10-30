(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Profiling : sig
  type t

  val record_stats :
    stage:string -> metric:string -> value:float -> profiling:t -> unit
end

val collect_cgroup_stats :
  profiling:Profiling.t -> stage:string -> f:(unit -> 'a) -> 'a

val profile_memory : event:string -> f:(Profiling.t -> 'a) -> Profiling.t * 'a

val log_to_scuba : stage:string -> profiling:Profiling.t -> unit

val print_summary_memory_table : Profiling.t -> unit
