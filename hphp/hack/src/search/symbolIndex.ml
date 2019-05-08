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

(* Note that fuzzy search does not currently do anything *)
let fuzzy_search_enabled () = !HackSearchService.fuzzy
let set_fuzzy_search_enabled x = HackSearchService.fuzzy := x

(* Keep track of current search provider in a global *)
let current_search_provider = ref None

(* If we are debugging locally, avoid logging anything - it makes tests fail *)
let in_quiet_mode = ref false

(* Fetch the currently selected search provider *)
let get_search_provider (): SearchUtils.search_provider =
  match !current_search_provider with
  | Some provider ->
    provider
  | None -> failwith "Attempted to fetch search provider, but it is not yet set.";
;;

(* If a provider requires initialization, put it here *)
let initialize_provider
    (provider: SearchUtils.search_provider)
    (savedstate_file_opt: string option): unit =
  let _ = savedstate_file_opt in
  match provider with
  | SqliteIndex
  | AllLocalIndex
  | GleanApiIndex
  | GrepIndex
  | NoIndex
  | RipGrepIndex
  | TrieIndex ->
    ()

(* Set the currently selected search provider *)
let set_search_provider
    ~(quiet: bool)
    ~(provider_name: string)
    ~(savedstate_file_opt: string option): unit =
  let provider = SearchUtils.provider_of_string provider_name in
  match !current_search_provider with
  | None ->
    in_quiet_mode := quiet;
    current_search_provider := Some provider;
    initialize_provider provider savedstate_file_opt;
    if not !in_quiet_mode then begin
      Hh_logger.log "Search provider set to [%s] based on configuration value [%s]"
        (SearchUtils.descriptive_name_of_provider provider)
        provider_name;
    end;
  | Some existing_provider ->

    (* We don't yet support changing providers on the fly *)
    if existing_provider <> provider then begin
      failwith "We do not currently support switching providers after launch";
    end;
;;

(* Log information about calls to the symbol index service *)
let log_symbol_index_search
    ~(query_text: string)
    ~(max_results: int)
    ~(results: int)
    ~(kind_filter: si_kind option)
    ~(start_time: float)
    ~(context: autocomplete_type option)
    ~(caller: string): unit =

  (* In quiet mode we don't log anything to either scuba or console *)
  if !in_quiet_mode then begin
    ()
  end else begin

    (* Calculate duration *)
    let end_time = Unix.gettimeofday () in
    let duration = end_time -. start_time in

    (* Clean up strings *)
    let kind_filter_str = match kind_filter with
      | None -> "None"
      | Some kind -> show_si_kind kind
    in
    let actype_str = match context with
      | None -> "None"
      | Some actype -> show_autocomplete_type actype
    in
    let search_provider = descriptive_name_of_provider (get_search_provider ()) in

    (* Send information to remote logging system *)
    HackEventLogger.search_symbol_index
      ~query_text
      ~max_results
      ~results
      ~kind_filter:kind_filter_str
      ~duration
      ~actype:actype_str
      ~caller
      ~search_provider;
    ()
  end
;;

(*
 * Core search function
 * - Uses both global and local indexes
 * - Can search for specific kinds or all kinds
 * - Goal is to route ALL searches through one function for consistency
 *)
let find_matching_symbols
    ~(query_text: string)
    ~(max_results: int)
    ~(kind_filter: si_kind option): si_results =
  let provider = get_search_provider () in
  (*
   * Nuclide often sends this exact request to verify that HH is working.
   * Let's capture it and avoid doing unnecessary work.
   *)
  if query_text = "this_is_just_to_check_liveness_of_hh_server" then begin
    [{
      si_name = "Yes_hh_server_is_alive";
      si_kind = SI_Unknown;
    }]
  end else begin

    (* The local index captures symbols in files that have been changed on disk.
     * Search it first for matches, then search global and add any elements
     * that we haven't seen before *)
    let local_results = match provider with
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
    let global_results = match provider with
      | AllLocalIndex
      | GleanApiIndex
      | GrepIndex
      | NoIndex
      | RipGrepIndex
      | SqliteIndex ->
        []
      | TrieIndex ->
        HackSearchService.index_search query_text max_results kind_filter
    in

    (* TODO: Deduplicate any items where the global result duplicates the local result *)
    List.append local_results global_results
  end
;;

(*
 * Legacy API
 * Will be replaced in a future diff in this stack
 *)
let query_for_symbol_search
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
 * This method is called when the typechecker has finished re-checking a file,
 * or when the saved-state is fully loaded.  Any system that needs to cache
 * this information should capture it here.
 *)
let update
    (worker_list_opt: MultiWorker.worker list option)
    (files: (Relative_path.t * info) list): unit =
  match get_search_provider () with
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
  match get_search_provider () with
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
