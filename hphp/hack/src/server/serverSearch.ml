(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_core
module SS = SearchUtils
module SUtils = SearchUtils

let result_to_json res =
  let desc_string = SearchUtils.kind_to_string res.SUtils.result_type in
  let p = res.SUtils.pos in
  let fn = Pos.filename p in
  let (line, start, end_) = Pos.info_pos p in
  Hh_json.JSON_Object
    [
      ("name", Hh_json.JSON_String (Utils.strip_ns res.SUtils.name));
      ("filename", Hh_json.JSON_String fn);
      ("desc", Hh_json.JSON_String desc_string);
      ("line", Hh_json.int_ line);
      ("char_start", Hh_json.int_ start);
      ("char_end", Hh_json.int_ end_);
      ("scope", Hh_json.JSON_String "");
    ]

let re_colon_colon = Str.regexp "::"

let go workers query type_ (sienv : SearchUtils.si_env) : SearchUtils.result =
  let max_results = 100 in
  let start_time = Unix.gettimeofday () in
  let kind_filter = SearchUtils.string_to_kind type_ in
  let fuzzy = SymbolIndex.fuzzy_search_enabled () in
  let context = Some SearchUtils.Ac_no_namespace in
  let results =
    (* If query contains "::", search class methods instead of top level definitions *)
    match Str.split_delim re_colon_colon query with
    | [class_name_query; method_query] ->
      (* Fixup the kind filter *)
      let kind_filter = Some SearchUtils.SI_Class in
      (* Get the class with the most similar name to `class_name_query` *)
      let class_ =
        (* Switch between old behavior and new *)
        match sienv.SearchUtils.sie_provider with
        | SearchUtils.TrieIndex ->
          SymbolIndex.query_for_symbol_search
            ~fuzzy
            workers
            class_name_query
            type_
          |> List.find ~f:(fun result ->
                 match result with
                 | SearchUtils.{ result_type = SearchUtils.SI_Trait; _ } ->
                   true
                 | SearchUtils.{ result_type = SearchUtils.SI_Enum; _ } -> true
                 | SearchUtils.{ result_type = SearchUtils.SI_Interface; _ } ->
                   true
                 | SearchUtils.{ result_type = SearchUtils.SI_Class; _ } ->
                   true
                 | _ -> false)
          |> Option.map ~f:(fun s -> s.SearchUtils.name)
        | _ ->
          SymbolIndex.find_matching_symbols
            ~query_text:class_name_query
            ~max_results:1
            ~kind_filter
            ~context
            ~sienv
          |> List.hd
          |> Option.map ~f:(fun r -> r.SearchUtils.si_name)
      in
      begin
        match class_ with
        | Some name ->
          ClassMethodSearch.query_class_methods
            (Utils.add_ns name)
            method_query
        | None ->
          (* When we can't find a class with a name similar to the given one,
           just return no search results. *)
          []
      end
    | _ ->
      (* Switch between old behavior and new *)
      (match sienv.SearchUtils.sie_provider with
      | SearchUtils.TrieIndex ->
        let temp_results =
          SymbolIndex.query_for_symbol_search ~fuzzy workers query type_
        in
        List.map temp_results SearchUtils.to_absolute
      | _ ->
        let temp_results =
          SymbolIndex.find_matching_symbols
            ~query_text:query
            ~max_results
            ~kind_filter
            ~context
            ~sienv
        in
        AutocompleteService.add_position_to_results temp_results)
  in
  SymbolIndex.log_symbol_index_search
    ~sienv
    ~start_time
    ~query_text:query
    ~max_results
    ~kind_filter
    ~results:(List.length results)
    ~context
    ~caller:"ServerSearch.go";
  results
