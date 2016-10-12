(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

(*****************************************************************************)
(* Recheck loop types. *)
(*****************************************************************************)

type recheck_loop_stats = {
  (** Watchman subscription has gone down, so state of the world after the
   * recheck loop may not reflect what is actually on disk. *)
  updates_stale : bool;
  rechecked_batches : int;
  rechecked_count : int;
  (* includes dependencies *)
  total_rechecked_count : int;
}

let empty_recheck_loop_stats = {
  updates_stale = false;
  rechecked_batches = 0;
  rechecked_count = 0;
  total_rechecked_count = 0;
}

(*****************************************************************************)
(* The "static" environment, initialized first and then doesn't change *)
(*****************************************************************************)

type genv = {
    options          : ServerArgs.options;
    config           : ServerConfig.t;
    local_config     : ServerLocalConfig.t;
    workers          : Worker.t list option;
    (* Returns the list of files under .hhconfig, subject to a filter *)
    indexer          : (string -> bool) -> string MultiWorker.nextlist;
    (* Each time this is called, it should return the files that have changed
     * since the last invocation *)
    notifier_async   : unit -> ServerNotifierTypes.notifier_changes;
    notifier         : unit -> SSet.t;
    (* If daemons are spawned as part of the init process, wait for them here *)
    wait_until_ready : unit -> unit;
    mutable debug_channels   : (Timeout.in_channel * out_channel) option;
  }

(*****************************************************************************)
(* The environment constantly maintained by the server *)
(*****************************************************************************)

(* In addition to this environment, many functions are storing and
 * updating ASTs, NASTs, and types in a shared space
 * (see respectively Parser_heap, Naming_heap, Typing_env).
 * The Ast.id are keys to index this shared space.
 *)
type env = {
    files_info     : FileInfo.t Relative_path.Map.t;
    tcopt          : TypecheckerOptions.t;
    popt           : ParserOptions.t;
    errorl         : Errors.t;
    (* Keeps list of files containing parsing errors in the last iteration. *)
    failed_parsing : Relative_path.Set.t;
    failed_decl    : Relative_path.Set.t;
    failed_check   : Relative_path.Set.t;
    persistent_client : ClientProvider.client option;
    (* Timestamp of last IDE file synchronization command *)
    last_command_time : float;
    (* The map from full path to synchronized file contents *)
    edited_files   : File_content.t Relative_path.Map.t;
    (* Files which parse trees were invalidated (because they changed on disk
     * or in editor) and need to be re-parsed *)
    ide_needs_parsing : Relative_path.Set.t;
    disk_needs_parsing : Relative_path.Set.t;
    (* Definitions that became invalidated and removed from heap. Depending
     * on lazy decl to update them on as-needed basis. Things that require
     * entire global state to be up to date (like global list of errors, build,
     * or find all references) must be preceded by Full_check. *)
    needs_decl : Relative_path.Set.t;
    needs_check : Relative_path.Set.t;
    needs_full_check : bool;
    (* The diagnostic subscription information of the current client *)
    diag_subscribe : Diagnostic_subscription.t option;
    recent_recheck_loop_stats : recheck_loop_stats;
  }

let file_filter f =
  (FindUtils.is_php f && not (FilesToIgnore.should_ignore f))
  || FindUtils.is_js f

let list_files env oc =
  let acc = List.fold_right
    ~f:begin fun error acc ->
      let pos = Errors.get_pos error in
      Relative_path.Set.add acc (Pos.filename pos)
    end
    ~init:Relative_path.Set.empty
    (Errors.get_error_list env.errorl) in
  Relative_path.Set.iter acc (fun s ->
    Printf.fprintf oc "%s\n" (Relative_path.to_absolute s));
  flush oc
