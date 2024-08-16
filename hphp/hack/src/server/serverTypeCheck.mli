(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module CheckStats : sig
  type t = {
    reparse_count: int;
    total_rechecked_count: int;
    time_first_result: ServerEnv.seconds_since_epoch option;
        (** This is either the duration to get the first error if any
            or until we get "typecheck done" status message. *)
  }
end

val type_check :
  ServerEnv.genv ->
  ServerEnv.env ->
  float ->
  CgroupProfiler.step_group ->
  ServerEnv.env * CheckStats.t * Telemetry.t

(****************************************************************************)
(* Debugging: Declared here to stop ocamlc yelling at us for unused defs *)
(****************************************************************************)

val print_defs : string -> ('a * string) list -> unit

val print_defs_per_file_pos :
  (('a * string) list * ('b * string) list) SMap.t -> unit

val print_fast : (SSet.t * SSet.t) SMap.t -> unit
