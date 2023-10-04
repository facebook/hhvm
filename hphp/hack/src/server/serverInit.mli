(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type load_state_approach =
  | Precomputed of ServerArgs.saved_state_target_info
  (* Load a saved state using Ocaml implementation of saved state loader. *)
  | Load_state_natively

type init_approach =
  | Full_init
  | Parse_only_init
  | Saved_state_init of load_state_approach
  | Write_symbol_info
  | Write_symbol_info_with_state of load_state_approach

(** Saves the state that is used by init below and returns the number of
    edges added to the saved state dependency table. *)
val save_state :
  ServerEnv.genv ->
  ServerEnv.env ->
  string ->
  SaveStateServiceTypes.save_state_result option

type init_result =
  (* Loaded a saved saved state of this distance. Note: for older load scripts
   * distance is unknown, thus None. *)
  | Load_state_succeeded of ServerEnv.saved_state_delta option
  (* Loading error *)
  | Load_state_failed of string * Telemetry.t
  (* This option means we didn't even try to load a saved state *)
  | Load_state_declined of string

(** Parse, name, typecheck the next set of files,
    refresh the environment and update the many shared heaps *)
val init :
  init_approach:init_approach ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ServerEnv.env * (* If the script failed, the error message *) init_result
