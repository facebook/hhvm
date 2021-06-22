(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type load_state_error =
  (* an error reported when downloading saved-state through [Saved_state_loader] *)
  | Load_state_saved_state_loader_failure of Saved_state_loader.load_error
  (* an error fetching list of dirty files from hg *)
  | Load_state_dirty_files_failure of Future.error
  (* either the downloader or hg-dirty-files took too long *)
  | Load_state_timeout
  (* any other unhandled exception from lazy_init *)
  | Load_state_unhandled_exception of {
      exn: exn;
      stack: Utils.callstack;
    }

type load_state_verbose_error = {
  message: string;
  stack: Utils.callstack;
  auto_retry: bool;
  environment: string option;
}
[@@deriving show]

type load_state_approach =
  | Precomputed of ServerArgs.saved_state_target_info
  | Load_state_natively
[@@deriving show]

type remote_init = {
  worker_key: string;
  nonce: Int64.t;
  check_id: string;
}
[@@deriving show]

type init_approach =
  | Full_init
  | Parse_only_init
  | Saved_state_init of load_state_approach
  | Remote_init of remote_init
  | Write_symbol_info
[@@deriving show]

(** Docs are in .mli *)
type init_result =
  | Load_state_succeeded of int option
  | Load_state_failed of string * Utils.callstack
  | Load_state_declined of string

(** returns human-readable string, an indication of whether auto-retry is sensible, and stack *)
let load_state_error_to_verbose_string (err : load_state_error) :
    load_state_verbose_error =
  match err with
  | Load_state_saved_state_loader_failure err ->
    (* TODO(hverr): Construct verbose errors with all fields properly set *)
    {
      message =
        Printf.sprintf
          ( "Could not load saved-state from DevX infrastructure. "
          ^^ "The underlying error message was: %s\n\n"
          ^^ "The accompanying debug details are: %s" )
          (Saved_state_loader.long_user_message_of_error err)
          (Saved_state_loader.debug_details_of_error err);
      auto_retry = false;
      stack = Utils.Callstack "";
      environment = None;
    }
  | Load_state_dirty_files_failure error ->
    let Future.{ message; stack; environment } =
      Future.error_to_string_verbose error
    in
    {
      message = Printf.sprintf "Problem getting dirty files from hg: %s" message;
      auto_retry = false;
      stack;
      environment;
    }
  | Load_state_timeout ->
    {
      message = "Timed out trying to load state";
      auto_retry = false;
      stack = Utils.Callstack "";
      environment = None;
    }
  | Load_state_unhandled_exception { exn; stack } ->
    {
      message =
        Printf.sprintf
          "Unexpected bug loading saved state - %s"
          (Printexc.to_string exn);
      auto_retry = false;
      stack;
      environment = None;
    }

type files_changed_while_parsing = Relative_path.Set.t

type loaded_info = {
  naming_table_fn: string;
  deptable_fn: string;
  deptable_is_64bit: bool;
  naming_table_fallback_fn: string option;
  corresponding_rev: Hg.rev;
  mergebase_rev: Hg.global_rev option;
  mergebase: Hg.hg_rev option Future.t; [@opaque]
  (* Files changed between the loaded naming table saved state and current revision. *)
  dirty_naming_files: Relative_path.Set.t; [@printer Relative_path.Set.pp_large]
  (* Files changed between saved state revision and current public merge base *)
  dirty_master_files: Relative_path.Set.t; [@printer Relative_path.Set.pp_large]
  (* Files changed between public merge base and current revision *)
  dirty_local_files: Relative_path.Set.t; [@printer Relative_path.Set.pp_large]
  old_naming_table: Naming_table.t; [@opaque]
  old_errors: SaveStateServiceTypes.saved_state_errors; [@opaque]
  state_distance: int option;
  (* The manifold path for naming table saved state, to be used by remote type checker
     for downloading the naming table in the case of a saved-state init *)
  naming_table_manifold_path: string option;
}
[@@deriving show]

(* Laziness *)
type lazy_level =
  | Off
  | Decl
  | Parse
  | Init
[@@deriving show]
