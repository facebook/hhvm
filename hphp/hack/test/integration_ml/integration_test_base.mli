(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types

val setup_server :
  ?custom_config:ServerConfig.t ->
  ?hhi_files:(string * string) list ->
  unit ->
  ServerEnv.env

val setup_disk : ServerEnv.env -> disk_changes_type -> ServerEnv.env

val change_files :
  ServerEnv.env -> disk_changes_type -> ServerEnv.env * ('a, unit) loop_outputs

val save_state :
  ?load_hhi_files:bool ->
  ?store_decls_in_saved_state:bool ->
  ?enable_naming_table_fallback:bool ->
  disk_changes_type ->
  string ->
  unit

val save_state_incremental :
  ServerEnv.env ->
  ?store_decls_in_saved_state:bool ->
  string ->
  SaveStateServiceTypes.save_state_result option

val save_state_with_errors : disk_changes_type -> string -> string -> unit

val load_state :
  ?master_changes:string list ->
  ?local_changes:string list ->
  ?load_hhi_files:bool ->
  ?use_precheked_files:bool ->
  ?disable_conservative_redecl:bool ->
  ?predeclare_ide_deps:bool ->
  ?load_decls_from_saved_state:bool ->
  ?enable_naming_table_fallback:bool ->
  disk_state:disk_changes_type ->
  string (* saved_state_dir *) ->
  ServerEnv.env

val in_daemon : (unit -> unit) -> unit

val connect_persistent_client : ServerEnv.env -> ServerEnv.env

val default_loop_input : ('a, 'b) loop_inputs

val run_loop_once :
  ServerEnv.env ->
  ('a, 'b) loop_inputs ->
  ServerEnv.env * ('a, 'b) loop_outputs

(* wrappers around run_loop_once for most common operations *)

val subscribe_diagnostic : ?id:int -> ServerEnv.env -> ServerEnv.env

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

val coverage_levels :
  ServerEnv.env ->
  ServerCommandTypes.file_input ->
  ServerEnv.env * ('a, Coverage_level_defs.result) loop_outputs

val coverage_counts :
  ServerEnv.env ->
  string ->
  ServerEnv.env * ('a, ServerCoverageMetricTypes.result) loop_outputs

val autocomplete :
  ServerEnv.env ->
  string ->
  ServerEnv.env * ('a, AutocompleteTypes.result) loop_outputs

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

val full_check : ServerEnv.env -> ServerEnv.env * ('a, unit) loop_outputs

val start_initial_full_check : ServerEnv.env -> ServerEnv.env * int

val prepend_root : string -> string

val errors_to_string : Pos.absolute Errors.error_ list -> string

(* Helpers for asserting things *)

val fail : string -> unit

val assertEqual : string -> string -> unit

val assert_no_errors : ServerEnv.env -> unit

val assert_errors : Errors.t -> string -> unit

val assert_env_errors : ServerEnv.env -> string -> unit

val assert_errors_in_phase :
  ServerEnv.env -> int -> Errors.phase -> ServerEnv.env

val assertSingleError : string -> Errors.error list -> unit

val assert_no_diagnostics : ('a, 'b) loop_outputs -> unit

val assert_has_diagnostics : ('a, 'b) loop_outputs -> unit

val assert_diagnostics : ('a, 'b) loop_outputs -> string -> unit

val assert_diagnostics_in : ('a, 'b) loop_outputs -> string -> string -> unit

val get_diagnostics :
  ('a, 'b) loop_outputs -> Pos.absolute Errors.error_ list SMap.t

val assert_coverage_levels :
  ('a, Coverage_level_defs.result) loop_outputs -> string list -> unit

val assert_coverage_counts :
  ('a, ServerCoverageMetricTypes.result) loop_outputs -> string list -> unit

val assert_autocomplete :
  ('a, AutocompleteTypes.result) loop_outputs -> string list -> unit

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

val assert_ide_find_refs :
  ('a, ServerCommandTypes.Find_refs.ide_result_or_retry) loop_outputs ->
  string ->
  string list ->
  unit

val assert_refactor :
  ('a, ServerCommandTypes.Refactor.result_or_retry) loop_outputs ->
  string ->
  unit

val assert_ide_refactor :
  ('a, ServerCommandTypes.Refactor.ide_result_or_retry) loop_outputs ->
  string ->
  unit

val assert_needs_recheck : ServerEnv.env -> string -> unit

val assert_needs_no_recheck : ServerEnv.env -> string -> unit
