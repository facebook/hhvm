(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open SearchUtils
open SearchTypes

(* Set the currently selected search provider *)
let initialize
    ~(gleanopt : GleanOptions.t)
    ~(namespace_map : (string * string) list)
    ~(provider_name : string)
    ~(quiet : bool) : si_env =
  (* Create the object *)
  let sienv =
    {
      SearchUtils.default_si_env with
      sie_provider = SearchUtils.provider_of_string provider_name;
      sie_quiet_mode = quiet;
      sie_namespace_map = namespace_map;
      glean_reponame = GleanOptions.reponame gleanopt;
    }
  in
  (* Basic initialization *)
  let sienv =
    match sienv.sie_provider with
    | CustomIndex -> CustomSearchService.initialize ~sienv
    | NoIndex
    | MockIndex _
    | LocalIndex ->
      sienv
  in

  (* Here's the initialized environment *)
  if not sienv.sie_quiet_mode then
    Hh_logger.log
      "Search provider set to [%s] based on configuration value [%s]"
      (SearchUtils.descriptive_name_of_provider sienv.sie_provider)
      provider_name;
  sienv

let mock ~on_find =
  {
    SearchUtils.default_si_env with
    sie_provider = SearchUtils.MockIndex { mock_on_find = on_find };
    sie_quiet_mode = true;
  }

(*
 * Core search function
 * - Uses both global and local indexes
 * - Can search for specific kinds or all kinds
 * - Goal is to route ALL searches through one function for consistency
 *)
let find_matching_symbols
    ~(sienv_ref : si_env ref)
    ~(query_text : string)
    ~(max_results : int)
    ~(context : autocomplete_type)
    ~(kind_filter : si_kind option) :
    SearchTypes.si_item list * SearchTypes.si_complete =
  let is_complete = ref SearchTypes.Complete in
  (*
   * Nuclide often sends this exact request to verify that HH is working.
   * Let's capture it and avoid doing unnecessary work.
   *)
  if String.equal query_text "this_is_just_to_check_liveness_of_hh_server" then
    ( [
        {
          si_name = "Yes_hh_server_is_alive";
          si_kind = SI_Unknown;
          si_file = SI_Filehash "0";
          si_fullname = "";
        };
      ],
      !is_complete )
  else
    (* Namespace aliases. For instance, if File is an alias for \FlibSL\File,
       then "Fi|" will definitely show File. *)
    let namespace_results =
      match context with
      | Ac_workspace_symbol -> []
      | Acid
      | Acnew
      | Acclassish
      | Actype
      | Actrait_only ->
        List.filter_map !sienv_ref.sie_namespace_map ~f:(fun (alias, map) ->
            if String.is_prefix alias ~prefix:query_text then
              Some
                {
                  si_name = alias;
                  si_kind = SI_Namespace;
                  si_file = SI_Filehash "0";
                  si_fullname = map;
                }
            else
              None)
    in
    (* The local index captures symbols in files that have been changed on disk.
     * Search it first for matches, then search global and add any elements
     * that we haven't seen before *)
    let local_results =
      match !sienv_ref.sie_provider with
      | NoIndex
      | MockIndex _ ->
        []
      | CustomIndex
      | LocalIndex ->
        LocalSearchService.search_local_symbols
          ~sienv:!sienv_ref
          ~query_text
          ~max_results
          ~context
          ~kind_filter
    in
    (* Next search globals *)
    let global_results =
      match !sienv_ref.sie_provider with
      | CustomIndex ->
        let (r, custom_is_complete) =
          CustomSearchService.search_symbols
            ~sienv_ref
            ~query_text
            ~max_results
            ~context
            ~kind_filter
        in
        is_complete := custom_is_complete;
        LocalSearchService.extract_dead_results ~sienv:!sienv_ref ~results:r
      | MockIndex { mock_on_find } ->
        mock_on_find ~query_text ~context ~kind_filter
      | LocalIndex
      | NoIndex ->
        []
    in
    (* Merge and deduplicate results *)
    let all_results = List.append local_results global_results in
    let dedup_results =
      List.sort
        ~compare:(fun a b -> String.compare b.si_name a.si_name)
        all_results
    in
    (* Strip namespace already typed from the results *)
    let (ns, _) = Utils.split_ns_from_name query_text in
    let results =
      if String.equal ns "" || String.equal ns "\\" then
        dedup_results
      else
        List.map dedup_results ~f:(fun s ->
            { s with si_name = String_utils.lstrip s.si_name ns })
    in
    (* Namespaces should always appear first *)
    let results = List.append namespace_results results in
    (List.take results max_results, !is_complete)

let find_refs
    ~(sienv_ref : si_env ref)
    ~(action : SearchTypes.Find_refs.action)
    ~(max_results : int) : Relative_path.t list option =
  match !sienv_ref.sie_provider with
  | NoIndex
  | LocalIndex
  | MockIndex _ ->
    None
  | CustomIndex -> CustomSearchService.find_refs ~sienv_ref ~action ~max_results
