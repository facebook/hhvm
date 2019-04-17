(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open SearchUtils

(* Select the current search provider with this compile-time constant *)
let current_search_provider = TrieIndex

(* Note that fuzzy search does not currently do anything *)
let fuzzy_search_enabled () = !HackSearchService.fuzzy
let set_fuzzy_search_enabled x = HackSearchService.fuzzy := x


(* Display the currently selected search provider *)
let log_search_provider (): unit =
  match current_search_provider with
  | SqliteIndex -> Hh_logger.log "Symbol index provider: Sqlite";
  | GrepIndex -> Hh_logger.log "Symbol index provider: GrepIndex";
  | RipGrepIndex -> Hh_logger.log "Symbol index provider: RipGrep";
  | NoIndex -> Hh_logger.log "Symbol index provider: None";
  | AllLocalIndex -> Hh_logger.log "Symbol index provider: All-Local";
  | GleanApiIndex -> Hh_logger.log "Symbol index provider: Glean API";
  | TrieIndex -> Hh_logger.log "Symbol index provider: SharedMem/Trie";
    ()
;;

(*
 * Core search function
 * - Uses both global and local indexes
 * - Can search for specific kinds or all kinds
 * - Goal is to route ALL searches through one function for consistency
 *)
let symbol_index_query
    (prefix: string)
    (max_results: int)
    (kind_opt: si_kind option): si_results =

  (*
   * Nuclide often sends this exact request to verify that HH is working.
   * Let's capture it and avoid doing unnecessary work.
   *)
  if prefix = "this_is_just_to_check_liveness_of_hh_server" then begin
    [{
      si_name = "Yes_hh_server_is_alive";
      si_kind = SI_Unknown;
    }]
  end else begin

    (* The local index captures symbols in files that have been changed on disk.
     * Search it first for matches, then search global and add any elements
     * that we haven't seen before *)
    let local_results = match current_search_provider with
      | AllLocalIndex
      | GleanApiIndex
      | GrepIndex
      | NoIndex
      | RipGrepIndex
      | SqliteIndex
      | TrieIndex ->
        [] (* This will be filled in by later commits *)
    in

    (* Next search globals *)
    let global_results = match current_search_provider with
      | AllLocalIndex
      | GleanApiIndex
      | GrepIndex
      | NoIndex
      | RipGrepIndex
      | SqliteIndex ->
        []
      | TrieIndex ->
        HackSearchService.index_search prefix max_results kind_opt
    in

    (* Return a unified list *)
    (* TODO: Deduplicate any items where the global result duplicates the local result *)
    List.append local_results global_results
  end
;;

(*
 * Legacy API
 * Will be replaced in a future diff in this stack
 *)
let query
    (worker_list_opt: MultiWorker.worker list option)
    (s1: string)
    (s2: string)
    ~(fuzzy: bool): (Pos.t, search_result_type) term list =

  (* Just route to the trie-based search service for now *)
  HackSearchService.MasterApi.query worker_list_opt s1 s2 fuzzy
;;

(*
 * Legacy API
 * Exact replacement TBD
 *)
let query_for_autocomplete
    (s1: string)
    ~(limit: int option)
    ~(filter_map:(
        string ->
        string ->
        (FileInfo.pos, search_result_type) term ->
        'a option)): 'a list Utils.With_complete_flag.t =

  (* Just route to the trie-based search service for now *)
  HackSearchService.MasterApi.query_autocomplete s1 limit filter_map
;;

(*
 * Legacy API
 * Replaced by "index_search" with kind_opt set to "Some SI_Class"
 *)
let query_class_methods
    (s1: string)
    (s2: string): (Pos.t, search_result_type) term list =

  (* Just route to the trie-based search service for now *)
  HackSearchService.ClassMethods.query s1 s2
;;

(*
 * This method is called when the typechecker has finished re-checking a file,
 * or when the saved-state is fully loaded.  Any system that needs to cache
 * this information should capture it here.
 *)
let update
    (worker_list_opt: MultiWorker.worker list option)
    (files: (Relative_path.t * info) list): unit =
    match current_search_provider with
      | AllLocalIndex
      | GleanApiIndex
      | GrepIndex
      | NoIndex
      | RipGrepIndex
      | SqliteIndex ->
        ()
      | TrieIndex ->
        HackSearchService.update_from_typechecker worker_list_opt files
;;

(*
 * This method is called when the typechecker is about to re-check a file.
 * Any local caches should be cleared of values for this file.
 *)
let remove_files (paths: Relative_path.Set.t): unit =
  match current_search_provider with
    | AllLocalIndex
    | GleanApiIndex
    | GrepIndex
    | NoIndex
    | RipGrepIndex
    | SqliteIndex ->
      ()
    | TrieIndex ->
      HackSearchService.MasterApi.clear_shared_memory paths
;;
