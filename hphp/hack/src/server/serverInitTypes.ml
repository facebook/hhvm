(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error =
  (* an error reported by mk_state_future for downloading saved-state *)
  | Lazy_init_loader_failure of State_loader.error
  (* an error fetching list of dirty files from hg *)
  | Lazy_init_dirty_files_failure of Future.error
  (* either the downloader or hg-dirty-files took too long *)
  | Lazy_init_timeout
  (* any other unhandled exception from lazy_init *)
  | Lazy_init_unhandled_exception of {exn: exn; stack: Utils.callstack;}

type load_state_approach =
  | Precomputed of ServerArgs.saved_state_target_info
  | Load_state_natively of bool
  | Load_state_natively_with_target of ServerMonitorUtils.target_saved_state

(** Docs are in .mli *)
type init_result =
  | State_loaded of int option
  | State_load_failed of string
  | State_load_declined of string

let error_to_verbose_string (err: error) : string =
  match err with
  | Lazy_init_loader_failure err ->
    Printf.sprintf "Lazy init error downloading saved-state: %s"
      (State_loader.error_string_verbose err)
  | Lazy_init_dirty_files_failure error ->
    let ({Process_types.stack=Utils.Callstack stack; _}, _) = error in
    Printf.sprintf "Lazy init error querying hg for dirty files: %s\n%s"
      (Future.error_to_string error) stack
  | Lazy_init_timeout ->
    Printf.sprintf "Lazy init timeout"
  | Lazy_init_unhandled_exception {exn; stack=Utils.Callstack stack;} ->
    Printf.sprintf "Lazy init unhandled exception: %s\n%s"
      (Printexc.to_string exn) stack

type files_changed_while_parsing = Relative_path.Set.t

type loaded_info =
{
  saved_state_fn : string;
  corresponding_rev : Hg.rev;
  mergebase_rev : Hg.svn_rev option;
  (* Files changed between saved state revision and current public merge base *)
  dirty_master_files : Relative_path.Set.t;
  (* Files changed between public merge base and current revision *)
  dirty_local_files : Relative_path.Set.t;
  old_saved : FileInfo.saved_state_info;
  old_errors : SaveStateService.saved_state_errors;
  state_distance: int option;
}

(* Laziness *)
type lazy_level = Off | Decl | Parse | Init
