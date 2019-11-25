(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type load_state_error =
  (* an error reported by mk_state_future for downloading saved-state *)
  | Load_state_loader_failure of State_loader.error
  (* an error fetching list of dirty files from hg *)
  | Load_state_dirty_files_failure of Future.error
  (* either the downloader or hg-dirty-files took too long *)
  | Load_state_timeout
  (* any other unhandled exception from lazy_init *)
  | Load_state_unhandled_exception of {
      exn: exn;
      stack: Utils.callstack;
    }

type load_state_approach =
  | Precomputed of ServerArgs.saved_state_target_info
  | Load_state_natively of bool
  | Load_state_natively_with_target of ServerMonitorUtils.target_saved_state

type remote_init = {
  worker_key: string;
  check_id: string;
}

type init_approach =
  | Full_init
  | Parse_only_init
  | Saved_state_init of load_state_approach
  | Remote_init of remote_init
  | Write_symbol_info

(** Docs are in .mli *)
type init_result =
  | Load_state_succeeded of int option
  | Load_state_failed of string
  | Load_state_declined of string

(** returns human-readable string, an indication of whether auto-retry is sensible, and stack *)
let load_state_error_to_verbose_string (err : load_state_error) :
    string * bool * Utils.callstack =
  match err with
  | Load_state_loader_failure err -> State_loader.error_string_verbose err
  | Load_state_dirty_files_failure error ->
    let (msg, stack) = Future.error_to_string_verbose error in
    (Printf.sprintf "Problem getting dirty files from hg: %s" msg, false, stack)
  | Load_state_timeout ->
    ("Timed out trying to load state", false, Utils.Callstack "")
  | Load_state_unhandled_exception { exn; stack } ->
    ( Printf.sprintf
        "Unexpected bug loading saved state - %s"
        (Printexc.to_string exn),
      false,
      stack )

type files_changed_while_parsing = Relative_path.Set.t

type loaded_info = {
  saved_state_fn: string;
  deptable_fn: string;
  naming_table_fn: string option;
  corresponding_rev: Hg.rev;
  mergebase_rev: Hg.global_rev option;
  mergebase: Hg.hg_rev option Future.t;
  (* Files changed between the loaded naming table saved state and current revision. *)
  dirty_naming_files: Relative_path.Set.t;
  (* Files changed between saved state revision and current public merge base *)
  dirty_master_files: Relative_path.Set.t;
  (* Files changed between public merge base and current revision *)
  dirty_local_files: Relative_path.Set.t;
  old_naming_table: Naming_table.t;
  old_errors: SaveStateServiceTypes.saved_state_errors;
  state_distance: int option;
}

(* Laziness *)
type lazy_level =
  | Off
  | Decl
  | Parse
  | Init
