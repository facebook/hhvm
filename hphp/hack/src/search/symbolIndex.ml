(*
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

let init_needs_search_updates ~(provider_name : string) : bool =
  match SearchUtils.provider_of_string provider_name with
  | LocalIndex
  | TrieIndex ->
    true
  | _ -> false

(* Set the currently selected search provider *)
let initialize
    ~(globalrev : int option)
    ~(gleanopt : GleanOptions.t)
    ~(namespace_map : (string * string) list)
    ~(provider_name : string)
    ~(quiet : bool)
    ~(savedstate_file_opt : string option)
    ~(workers : MultiWorker.worker list option) : si_env =
  (* Create the object *)
  let sienv =
    {
      SearchUtils.default_si_env with
      sie_provider = SearchUtils.provider_of_string provider_name;
      sie_quiet_mode = quiet;
    }
  in
  (* Basic initialization *)
  let sienv =
    match sienv.sie_provider with
    | SqliteIndex ->
      SqliteSearchService.initialize ~sienv ~workers ~savedstate_file_opt
    | CustomIndex ->
      CustomSearchService.initialize ~globalrev ~gleanopt;
      sienv
    | NoIndex
    | LocalIndex
    | TrieIndex ->
      sienv
  in
  (* Fetch namespaces from provider-specific query *)
  let namespace_list =
    match sienv.sie_provider with
    | SqliteIndex -> SqliteSearchService.fetch_namespaces ~sienv
    | CustomIndex -> CustomSearchService.fetch_namespaces ()
    | NoIndex
    | LocalIndex
    | TrieIndex ->
      []
  in
  (* Register all namespaces *)
  List.iter namespace_list ~f:(fun namespace ->
      NamespaceSearchService.register_namespace ~sienv ~namespace);

  (* Add namespace aliases from the .hhconfig file *)
  List.iter namespace_map ~f:(fun (alias, target) ->
      NamespaceSearchService.register_alias ~sienv ~alias ~target);

  (* Here's the initialized environment *)
  if not sienv.sie_quiet_mode then
    Hh_logger.log
      "Search provider set to [%s] based on configuration value [%s]"
      (SearchUtils.descriptive_name_of_provider sienv.sie_provider)
      provider_name;
  sienv

(* Log information about calls to the symbol index service *)
let log_symbol_index_search
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(results : int)
    ~(kind_filter : si_kind option)
    ~(start_time : float)
    ~(context : autocomplete_type option)
    ~(caller : string) : unit =
  (* In quiet mode we don't log anything to either scuba or console *)
  if sienv.sie_quiet_mode then
    ()
  else
    (* Calculate duration *)
    let end_time = Unix.gettimeofday () in
    let duration = end_time -. start_time in
    (* Clean up strings *)
    let kind_filter_str =
      match kind_filter with
      | None -> "None"
      | Some kind -> show_si_kind kind
    in
    let actype_str =
      match context with
      | None -> "None"
      | Some actype -> show_autocomplete_type actype
    in
    let search_provider = descriptive_name_of_provider sienv.sie_provider in
    (* Send information to remote logging system *)
    if sienv.sie_log_timings then
      Hh_logger.log
        "[symbolindex] Search [%s] for [%s] [%s] found %d results in [%0.3f]"
        search_provider
        query_text
        actype_str
        results
        duration;
    HackEventLogger.search_symbol_index
      ~query_text
      ~max_results
      ~results
      ~kind_filter:kind_filter_str
      ~duration
      ~actype:actype_str
      ~caller
      ~search_provider

(*
 * Core search function
 * - Uses both global and local indexes
 * - Can search for specific kinds or all kinds
 * - Goal is to route ALL searches through one function for consistency
 *)
let find_matching_symbols
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(context : autocomplete_type option)
    ~(kind_filter : si_kind option) : si_results =
  (*
   * Nuclide often sends this exact request to verify that HH is working.
   * Let's capture it and avoid doing unnecessary work.
   *)
  if query_text = "this_is_just_to_check_liveness_of_hh_server" then
    [
      {
        si_name = "Yes_hh_server_is_alive";
        si_kind = SI_Unknown;
        si_filehash = 0L;
        si_fullname = "";
      };
    ]
  else
    (* Potential namespace matches always show up first *)
    let namespace_results =
      if context <> Some Ac_no_namespace then
        NamespaceSearchService.find_matching_namespaces ~sienv ~query_text
      else
        []
    in
    (* The local index captures symbols in files that have been changed on disk.
     * Search it first for matches, then search global and add any elements
     * that we haven't seen before *)
    let local_results =
      match sienv.sie_provider with
      | NoIndex
      | TrieIndex ->
        []
      | CustomIndex
      | LocalIndex
      | SqliteIndex ->
        LocalSearchService.search_local_symbols
          ~sienv
          ~query_text
          ~max_results
          ~context
          ~kind_filter
    in
    (* Next search globals *)
    let global_results =
      match sienv.sie_provider with
      | CustomIndex ->
        let r =
          CustomSearchService.search_symbols
            ~query_text
            ~max_results
            ~context
            ~kind_filter
        in
        LocalSearchService.extract_dead_results ~sienv ~results:r
      | SqliteIndex ->
        let results =
          SqliteSearchService.sqlite_search
            ~sienv
            ~query_text
            ~max_results
            ~context
            ~kind_filter
        in
        LocalSearchService.extract_dead_results ~sienv ~results
      | TrieIndex ->
        HackSearchService.index_search query_text max_results kind_filter
      | LocalIndex
      | NoIndex ->
        []
    in
    (* Merge and deduplicate results *)
    let all_results = List.append local_results global_results in
    let dedup_results =
      List.dedup_and_sort
        ~compare:(fun a b -> String.compare b.si_name a.si_name)
        all_results
    in
    (* Strip namespace already typed from the results *)
    let (ns, _) = Utils.split_ns_from_name query_text in
    let clean_results =
      if ns = "" || ns = "\\" then
        dedup_results
      else
        List.map dedup_results ~f:(fun s ->
            { s with si_name = String_utils.lstrip s.si_name ns })
    in
    (* Namespaces should always appear first *)
    let results = List.append namespace_results clean_results in
    List.take results max_results

(*
 * Legacy API
 * Will be replaced in a future diff in this stack
 *)
let query_for_symbol_search
    (worker_list_opt : MultiWorker.worker list option)
    (s1 : string)
    (s2 : string)
    ~(fuzzy : bool) : (Pos.t, si_kind) term list =
  (* Just route to the trie-based search service for now *)
  HackSearchService.MasterApi.query worker_list_opt s1 s2 fuzzy

(*
 * Legacy API
 * Exact replacement TBD
 *)
let query_for_autocomplete
    (s1 : string)
    ~(limit : int option)
    ~(filter_map :
       string -> string -> (FileInfo.pos, si_kind) term -> 'a option) :
    'a list Utils.With_complete_flag.t =
  (* Just route to the trie-based search service for now *)
  HackSearchService.MasterApi.query_autocomplete s1 limit filter_map

(*
 * This method is called when the typechecker has finished re-checking a file,
 * or when the saved-state is fully loaded.  Any system that needs to cache
 * this information should capture it here.
 *)
let update_files
    ~(sienv : si_env ref)
    ~(workers : MultiWorker.worker list option)
    ~(paths : (Relative_path.t * info * file_source) list) : unit =
  match !sienv.sie_provider with
  | NoIndex -> ()
  | CustomIndex
  | LocalIndex
  | SqliteIndex ->
    List.iter paths ~f:(fun (path, info, detector) ->
        if detector = SearchUtils.TypeChecker then
          sienv := LocalSearchService.update_file ~sienv:!sienv ~path ~info)
  | TrieIndex -> HackSearchService.update_from_typechecker workers paths

(*
 * This method is called when the typechecker is about to re-check a file.
 * Any local caches should be cleared of values for this file.
 *)
let remove_files
    ~(sienv : SearchUtils.si_env ref) ~(paths : Relative_path.Set.t) : unit =
  match !sienv.sie_provider with
  | NoIndex -> ()
  | CustomIndex
  | LocalIndex
  | SqliteIndex ->
    Relative_path.Set.iter paths ~f:(fun path ->
        sienv := LocalSearchService.remove_file ~sienv:!sienv ~path)
  | TrieIndex -> HackSearchService.MasterApi.clear_shared_memory paths

(* Fetch best available position information for a symbol *)
let get_position_for_symbol (symbol : string) (kind : si_kind) :
    (Relative_path.t * int * int) option =
  (* Symbols can only be found if they have a backslash *)
  let name_with_ns = Utils.add_ns symbol in
  let pos_opt =
    match kind with
    | SI_XHP
    | SI_Interface
    | SI_Trait
    | SI_Enum
    | SI_Typedef
    | SI_Class ->
      let fipos =
        match Naming_table.Types.get_pos name_with_ns with
        | None -> None
        | Some (pos, _) -> Some pos
      in
      fipos
    | SI_Function -> Naming_table.Funs.get_pos name_with_ns
    | SI_GlobalConstant -> Naming_table.Consts.get_pos name_with_ns
    (* Items below this are not global symbols and cannot be 'position'ed *)
    | SI_Unknown
    | SI_Namespace
    | SI_ClassMethod
    | SI_Literal
    | SI_ClassConstant
    | SI_Property
    | SI_LocalVariable
    | SI_Keyword
    | SI_Constructor
    | SI_Mixed ->
      None
  in
  (* Okay, we have a rough pos, convert that to a real pos *)
  match pos_opt with
  | None -> None
  | Some fi ->
    let (pos, _) = NamingGlobal.GEnv.get_full_pos (fi, name_with_ns) in
    let relpath = FileInfo.get_pos_filename fi in
    let (line, col, _) = Pos.info_pos pos in
    Some (relpath, line, col)

let absolute_none = Pos.none |> Pos.to_absolute

(* Shortcut to use the above method to get an absolute pos *)
let get_pos_for_item_opt (item : si_item) : Pos.absolute option =
  let result = get_position_for_symbol item.si_name item.si_kind in
  match result with
  | None -> None
  | Some (relpath, line, col) ->
    let symbol_len = String.length item.si_name in
    let pos =
      Pos.make_from_lnum_bol_cnum
        ~pos_file:relpath
        ~pos_start:(line, 1, col)
        ~pos_end:(line, 1, col + symbol_len)
    in
    Some (Pos.to_absolute pos)

(* Shortcut to use the above method to get an absolute pos *)
let get_pos_for_item (item : si_item) : Pos.absolute =
  let result = get_pos_for_item_opt item in
  match result with
  | None -> absolute_none
  | Some pos -> pos
