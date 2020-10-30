(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module MemStats : sig
  type memory_result

  type running

  type finished

  val sample_memory :
    group:string -> metric:string -> value:float -> running -> unit

  val log_to_scuba : stage:string -> running -> unit
end

val collect_cgroup_stats :
  MemStats.running -> group:string -> f:(unit -> 'a) -> 'a

val profile_memory :
  label:string -> f:(MemStats.running -> 'a) -> MemStats.finished * 'a

val print_summary_memory_table : MemStats.finished -> unit
