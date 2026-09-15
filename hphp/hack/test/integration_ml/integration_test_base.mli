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
module FileMap = S_map
module ErrorSet = SSet

val in_daemon : (unit -> unit) -> unit

val default_loop_input : 'a loop_inputs

val setup_disk : Server_env.env -> disk_changes_type -> Server_env.env

val change_files :
  Server_env.env -> disk_changes_type -> Server_env.env * 'a loop_outputs

val setup_server :
  ?custom_config:Server_config.t ->
  ?hhi_files:(string * string) list ->
  ?edges_dir:string ->
  unit ->
  Server_env.env

val run_loop_once :
  Server_env.env -> 'a loop_inputs -> Server_env.env * 'a loop_outputs

(* wrappers around run_loop_once for most common operations *)

val full_check_status : Server_env.env -> Server_env.env * 'a loop_outputs

val start_initial_full_check : Server_env.env -> Server_env.env * int

val prepend_root : string -> string

(** Some tests work with clientIdeDaemon rather than Server.
They use the following module instead of [setup_server] and [setup_disk]. *)
module Client : sig
  type env = Client_ide_daemon.Test.env

  val with_env : custom_config:Server_config.t option -> (env -> unit) -> unit

  val setup_disk : env -> (string * string) list -> env

  val open_file :
    env -> string -> env * Client_ide_message.diagnostic list S_map.t

  val close_file :
    env -> string -> env * Client_ide_message.diagnostic list S_map.t

  val edit_file :
    env -> string -> string -> env * Client_ide_message.diagnostic list S_map.t

  val assert_no_diagnostics : Client_ide_message.diagnostic list S_map.t -> unit

  val assert_diagnostics_string :
    Client_ide_message.diagnostic list S_map.t -> string -> unit
end

val doc :
  string (* file-suffix *) ->
  string (* content *) ->
  Client_ide_message.document

(* Helpers for asserting things *)

(** This helper is designed to make tests that work with TestDisk roots
like "/", or RealDisk roots like "/tmp/abc123/". It works by removing from
the string any occurrence of the global mutable root prefix
that was set by [Relative_path.set_prefix]. *)
val relativize : string -> string

val fail : string -> 'noreturn

val assertEqual : string -> string -> unit

val assert_no_diagnostics : Server_env.env -> unit

val assert_diagnostics : Diagnostics.t -> string -> unit

val assert_env_diagnostics : Server_env.env -> string -> unit

val assertSingleDiagnostic : string -> Diagnostics.diagnostic list -> unit

val diagnostic_strings : Diagnostics.diagnostic list -> string list

val assert_ide_completions :
  Autocomplete_types.ide_result -> string list -> unit

val assert_needs_retry :
  'a Server_command_types.Done_or_retry.t loop_outputs -> unit

val assert_find_refs :
  Server_command_types.Find_refs.result_or_retry loop_outputs ->
  string list ->
  unit

val assert_rename :
  Server_command_types.Rename.result_or_retry loop_outputs -> string -> unit

val assert_needs_recheck : Server_env.env -> string -> unit

val assert_needs_no_recheck : Server_env.env -> string -> unit
