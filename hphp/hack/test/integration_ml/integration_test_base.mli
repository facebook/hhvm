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

type error_messages_per_file = ErrorSet.t FileMap.t [@@deriving eq, show]

val setup_server :
  ?custom_config:ServerConfig.t ->
  ?hhi_files:(string * string) list ->
  ?edges_dir:string ->
  unit ->
  ServerEnv.env

val setup_disk : ServerEnv.env -> disk_changes_type -> ServerEnv.env

val change_files :
  ServerEnv.env -> disk_changes_type -> ServerEnv.env * ('a, unit) loop_outputs

val save_state :
  ?load_hhi_files:bool ->
  ?store_decls_in_saved_state:bool ->
  ?enable_naming_table_fallback:bool ->
  ?custom_config:ServerConfig.t ->
  disk_changes_type ->
  string ->
  unit

val save_state_incremental :
  ServerEnv.env ->
  ?store_decls_in_saved_state:bool ->
  old_state_dir:string ->
  string ->
  SaveStateServiceTypes.save_state_result option

val save_state_with_errors : disk_changes_type -> string -> string -> unit

val load_state :
  ?master_changes:string list ->
  ?local_changes:string list ->
  ?load_hhi_files:bool ->
  ?use_precheked_files:bool ->
  ?enable_naming_table_fallback:bool ->
  ?custom_config:ServerConfig.t ->
  disk_state:disk_changes_type ->
  string (* saved_state_dir *) ->
  ServerEnv.env

val in_daemon : (unit -> unit) -> unit

val connect_persistent_client : ServerEnv.env -> ServerEnv.env

val default_loop_input : ('a, 'b) loop_inputs

val run_loop_once :
  ServerEnv.env -> ('a, 'b) loop_inputs -> ServerEnv.env * ('a, 'b) loop_outputs

(* wrappers around run_loop_once for most common operations *)

val open_file : ServerEnv.env -> ?contents:string -> string -> ServerEnv.env

val edit_file :
  ServerEnv.env -> string -> string -> ServerEnv.env * ('a, unit) loop_outputs

val save_file :
  ServerEnv.env -> string -> string -> ServerEnv.env * ('a, unit) loop_outputs

val close_file :
  ?ignore_response:bool ->
  ServerEnv.env ->
  string ->
  ServerEnv.env * ('a, unit) loop_outputs

val wait : ServerEnv.env -> ServerEnv.env

val ide_autocomplete :
  ServerEnv.env ->
  string * int * int ->
  ServerEnv.env * ('a, AutocompleteTypes.ide_result) loop_outputs

val status :
  ?ignore_ide:bool ->
  ?max_errors:int option ->
  ?remote:bool ->
  ServerEnv.env ->
  ServerEnv.env * (ServerCommandTypes.Server_status.t, 'a) loop_outputs

val full_check_status : ServerEnv.env -> ServerEnv.env * ('a, unit) loop_outputs

val start_initial_full_check : ServerEnv.env -> ServerEnv.env * int

val prepend_root : string -> string

val errors_to_string : Errors.finalized_error list -> string

val print_telemetries : ServerEnv.env -> unit

(** Some tests work with clientIdeDaemon rather than Server.
They use the following module instead of [setup_server] and [setup_disk]. *)
module Client : sig
  type env = ClientIdeDaemon.Test.env

  val with_env : custom_config:ServerConfig.t option -> (env -> unit) -> unit

  val setup_disk : env -> (string * string) list -> env

  val open_file : env -> string -> env * ServerCommandTypes.diagnostic_errors

  val close_file : env -> string -> env * ServerCommandTypes.diagnostic_errors

  val edit_file :
    env -> string -> string -> env * ServerCommandTypes.diagnostic_errors

  val assert_no_diagnostics : ServerCommandTypes.diagnostic_errors -> unit

  val assert_diagnostics_string :
    ServerCommandTypes.diagnostic_errors -> string -> unit
end

val doc :
  string (* file-suffix *) -> string (* content *) -> ClientIdeMessage.document

val loc :
  int (* 1-based line *) ->
  int (* 1-based column *) ->
  ClientIdeMessage.location

(* Helpers for asserting things *)

(** This helper is designed to make tests that work with TestDisk roots
like "/", or RealDisk roots like "/tmp/abc123/". It works by removing from
the string any occurrence of the global mutable root prefix
that was set by [Relative_path.set_prefix]. *)
val relativize : string -> string

val fail : string -> 'noreturn

val assertEqual : string -> string -> unit

val assert_no_errors : ServerEnv.env -> unit

val assert_errors : Errors.t -> string -> unit

val assert_env_errors : ServerEnv.env -> string -> unit

val assertSingleError : string -> Errors.error list -> unit

val assert_no_diagnostics : ('a, 'b) loop_outputs -> unit

val assert_has_diagnostics : ('a, 'b) loop_outputs -> unit

val assert_diagnostics :
  ('a, 'b) loop_outputs -> error_messages_per_file -> unit

val assert_diagnostics_string : ('a, 'b) loop_outputs -> string -> unit

val assert_diagnostics_in :
  ('a, 'b) loop_outputs -> filename:string -> string -> unit

val get_diagnostics :
  ('a, 'b) loop_outputs -> Errors.finalized_error list SMap.t

val assert_ide_autocomplete_does_not_contain :
  ('a, AutocompleteTypes.ide_result) loop_outputs -> string list -> unit

val assert_ide_completions : AutocompleteTypes.ide_result -> string list -> unit

val assert_ide_autocomplete :
  ('a, AutocompleteTypes.ide_result) loop_outputs -> string list -> unit

val assert_status :
  (ServerCommandTypes.Server_status.t, 'a) loop_outputs -> string -> unit

val assert_error_count :
  (ServerCommandTypes.Server_status.t, 'a) loop_outputs ->
  expected_count:int ->
  unit

val assert_needs_retry :
  ('a, 'b ServerCommandTypes.Done_or_retry.t) loop_outputs -> unit

val assert_find_refs :
  ('a, ServerCommandTypes.Find_refs.result_or_retry) loop_outputs ->
  string list ->
  unit

val assert_rename :
  ('a, ServerCommandTypes.Rename.result_or_retry) loop_outputs -> string -> unit

val assert_ide_rename :
  ('a, ServerCommandTypes.Rename.ide_result_or_retry) loop_outputs ->
  string ->
  unit

val assert_needs_recheck : ServerEnv.env -> string -> unit

val assert_needs_no_recheck : ServerEnv.env -> string -> unit

val error_strings : Errors.error list -> string list
