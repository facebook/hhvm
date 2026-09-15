(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SUtils = Search_utils

let result_to_json res =
  let desc_string = Search_utils.kind_to_string res.SUtils.result_type in
  let p = res.SUtils.pos in
  let fn = Pos.filename p in
  let (line, start, end_) = Pos.info_pos p in
  `Assoc
    [
      ("name", `String (Utils.strip_ns res.SUtils.name));
      ("filename", `String fn);
      ("desc", `String desc_string);
      ("line", `Int line);
      ("char_start", `Int start);
      ("char_end", `Int end_);
      ("scope", `String "");
    ]

let re_colon_colon = Str.regexp "::"

let go
    ctx query_text ~(kind_filter : string) (sienv_ref : Search_utils.si_env ref)
    : Search_utils.result =
  let max_results = 100 in
  let start_time = Unix.gettimeofday () in
  let kind_filter = Search_utils.string_to_kind kind_filter in
  let context = Search_types.Ac_workspace_symbol in
  let results =
    (* If query contains "::", search class methods instead of top level definitions *)
    match Str.split_delim re_colon_colon query_text with
    | [class_name_query; method_query] ->
      (* Fixup the kind filter *)
      let kind_filter = Some File_info.SI_Class in
      (* Get the class with the most similar name to `class_name_query` *)
      let (candidates, _is_complete) =
        Symbol_index.find_matching_symbols
          ~query_text:class_name_query
          ~max_results:1
          ~kind_filter
          ~context
          ~sienv_ref
      in
      let class_ =
        candidates |> List.hd |> Option.map ~f:(fun r -> r.Search_types.si_name)
      in
      begin
        match class_ with
        | Some name ->
          Class_method_search.query_class_methods
            ctx
            (Utils.add_ns name)
            method_query
        | None ->
          (* When we can't find a class with a name similar to the given one,
             just return no search results. *)
          []
      end
    | _ ->
      let (temp_results, _is_complete) =
        Symbol_index.find_matching_symbols
          ~sienv_ref
          ~query_text
          ~max_results
          ~context
          ~kind_filter
      in
      Autocomplete_service.add_position_to_results ctx temp_results
  in
  Symbol_index_core.log_symbol_index_search
    ~sienv:!sienv_ref
    ~start_time
    ~query_text
    ~max_results
    ~kind_filter
    ~results:(List.length results)
    ~caller:"Ide_search.go";
  results
