(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Reordered_argument_collections
open Integration_test_base_types
module FileMap = SMap
module ErrorSet = SSet

val in_daemon : (unit -> unit) -> unit

val default_loop_input : 'a loop_inputs

val setup_disk : ServerEnv.env -> disk_changes_type -> ServerEnv.env

val change_files :
  ServerEnv.env -> disk_changes_type -> ServerEnv.env * 'a loop_outputs

val setup_server :
  ?custom_config:ServerConfig.t ->
  ?hhi_files:(string * string) list ->
  ?edges_dir:string ->
  unit ->
  ServerEnv.env

val run_loop_once :
  ServerEnv.env -> 'a loop_inputs -> ServerEnv.env * 'a loop_outputs

(* wrappers around run_loop_once for most common operations *)

val full_check_status : ServerEnv.env -> ServerEnv.env * 'a loop_outputs

val start_initial_full_check : ServerEnv.env -> ServerEnv.env * int

val prepend_root : string -> string

(** Some tests work with clientIdeDaemon rather than Server.
They use the following module instead of [setup_server] and [setup_disk]. *)
module Client : sig
  type env = ClientIdeDaemon.Test.env

  val with_env : custom_config:ServerConfig.t option -> (env -> unit) -> unit

  val setup_disk : env -> (string * string) list -> env

  val open_file : env -> string -> env * ClientIdeMessage.diagnostic list SMap.t

  val close_file :
    env -> string -> env * ClientIdeMessage.diagnostic list SMap.t

  val edit_file :
    env -> string -> string -> env * ClientIdeMessage.diagnostic list SMap.t

  val assert_no_diagnostics : ClientIdeMessage.diagnostic list SMap.t -> unit

  val assert_diagnostics_string :
    ClientIdeMessage.diagnostic list SMap.t -> string -> unit
end

val doc :
  string (* file-suffix *) -> string (* content *) -> ClientIdeMessage.document

(* Helpers for asserting things *)

(** This helper is designed to make tests that work with TestDisk roots
like "/", or RealDisk roots like "/tmp/abc123/". It works by removing from
the string any occurrence of the global mutable root prefix
that was set by [Relative_path.set_prefix]. *)
val relativize : string -> string

val fail : string -> 'noreturn

val assertEqual : string -> string -> unit

val assert_no_diagnostics : ServerEnv.env -> unit

val assert_diagnostics : Diagnostics.t -> string -> unit

val assert_env_diagnostics : ServerEnv.env -> string -> unit

val assertSingleDiagnostic : string -> Diagnostics.diagnostic list -> unit

val diagnostic_strings : Diagnostics.diagnostic list -> string list

val assert_ide_completions : AutocompleteTypes.ide_result -> string list -> unit

val assert_needs_retry :
  'a ServerCommandTypes.Done_or_retry.t loop_outputs -> unit

val assert_find_refs :
  ServerCommandTypes.Find_refs.result_or_retry loop_outputs ->
  string list ->
  unit

val assert_rename :
  ServerCommandTypes.Rename.result_or_retry loop_outputs -> string -> unit

val assert_needs_recheck : ServerEnv.env -> string -> unit

val assert_needs_no_recheck : ServerEnv.env -> string -> unit
