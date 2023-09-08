(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module CheckKind : sig
  type t =
    | Full
        (** Full check brings the global state of the server to consistency by
            executing all the re-checks that lazy checks delayed. It processes the
            disk updates and typechecks the full fanout of accumulated changes. *)

  (** This function is used to get the variant constructor names of
      the check kind type. The names are used in a few places:
      - the `type_check_unsafe` function below:
        - logs the names into the server log
        - uses HackEventLogger to log the names as the check_kind column value
        - lots of dashboards depend on it
      - serverMain writes it into telemetry
      *)
  val to_string : t -> string

  val is_full_check : t -> bool
end

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
  CheckKind.t ->
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
