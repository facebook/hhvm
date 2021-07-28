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
    | Lazy
        (** Lazy check is a check limited to the files open in IDE. It:
            - produces push diagnostics for those files
            - updates their parsing / naming / decl definitions on heap
            - updates their parsing level indexes, like SymbolIndex or
                ServerEnv.naming_table
            - invalidates their declaration dependencies, by removing them from the
                heap and depending on lazy declaration to redeclare them on
                as-needed basis later
            - stores the information about what it skipped doing to be finished later
                by Full

            It does not do the "full" expensive fanout:
            - does not re-declare dependencies ("phase 2 decl")
            - does not fan out to all typing dependencies
            - because of that, it does not update structures depending on global state,
                like global error list, dependency table or the lists of files that
                failed parsing / declaration / checking

            Any operation that need the global state to be up to date and cannot get
            the data that they need through lazy decl, need to be preceded by
            Full. *)
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
end

type check_results = {
  reparse_count: int;
  total_rechecked_count: int;
}

val type_check :
  ServerEnv.genv ->
  ServerEnv.env ->
  CheckKind.t ->
  float ->
  CgroupProfiler.Profiling.t ->
  ServerEnv.env * check_results * Telemetry.t

(****************************************************************************)
(* Debugging: Declared here to stop ocamlc yelling at us for unused defs *)
(****************************************************************************)

val print_defs : string -> ('a * string) list -> unit

val print_fast_pos : (('a * string) list * ('b * string) list) SMap.t -> unit

val print_fast : (SSet.t * SSet.t) SMap.t -> unit
