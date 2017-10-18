(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Integration_test_base_types

val setup_server: ?custom_config:ServerConfig.t ->
  ?hhi_files:(string * string) list -> unit -> ServerEnv.env

val setup_disk: ServerEnv.env -> disk_changes_type -> ServerEnv.env

val connect_persistent_client: ServerEnv.env -> ServerEnv.env

val default_loop_input: ('a, 'b) loop_inputs

val run_loop_once: ServerEnv.env -> ('a, 'b) loop_inputs ->
  (ServerEnv.env * ('a, 'b) loop_outputs)

(* wrappers around run_loop_once for most common operations *)

val subscribe_diagnostic : ?id:int -> ServerEnv.env  -> ServerEnv.env

val open_file : ServerEnv.env -> ?contents:string -> string -> ServerEnv.env

val edit_file :
  ServerEnv.env ->
  string ->
  string ->
  ServerEnv.env * ('a, unit) loop_outputs

val close_file :
  ServerEnv.env ->
  string ->
  ServerEnv.env * ('a, unit) loop_outputs

val wait :
  ServerEnv.env -> ServerEnv.env

val autocomplete :
  ServerEnv.env ->
  string ->
  ServerEnv.env * ('a, AutocompleteService.result) loop_outputs

val ide_autocomplete :
  ServerEnv.env ->
  (string * int * int) ->
  ServerEnv.env * ('a, AutocompleteService.ide_result) loop_outputs

val status :
  ?ignore_ide: bool ->
  ServerEnv.env ->
  ServerEnv.env *
    (ServerCommandTypes.Server_status.t, 'a) loop_outputs

val prepend_root: string -> string

val errors_to_string : Pos.absolute Errors.error_ list -> string

(* Helpers for asserting things *)

val fail: string -> unit

val assertEqual: string -> string -> unit

val assert_no_errors: ServerEnv.env -> unit

val assert_errors: ServerEnv.env -> string -> unit

val assertSingleError: string -> Errors.error list -> unit

val assert_no_diagnostics : ('a, 'b) loop_outputs -> unit

val assert_has_diagnostics : ('a, 'b) loop_outputs -> unit

val assert_diagnostics : ('a, 'b) loop_outputs -> string -> unit

val assert_autocomplete :
  ('a, AutocompleteService.result) loop_outputs -> string list -> unit

val assert_ide_autocomplete :
  ('a, AutocompleteService.ide_result) loop_outputs -> string list -> unit

val assert_status :
  (ServerCommandTypes.Server_status.t, 'a) loop_outputs -> string -> unit
