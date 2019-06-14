(**
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

let scope_string_from_type result_type =
  match result_type with
  | SS.Method (_, scope)
  | SS.ClassVar (_, scope) -> scope
  | _ -> ""

let desc_string_from_type result_type =
  match result_type with
  | SS.Class (Some class_kind) ->
    (match class_kind with
     | Ast.Cabstract -> "abstract class"
     | Ast.Cnormal -> "class"
     | Ast.Cinterface -> "interface"
     | Ast.Ctrait -> "trait"
     | Ast.Cenum -> "enum"
     | Ast.Crecord -> "record")
  (* This should never happen *)
  | SS.Class None ->  assert false
  | SS.Method (static, scope) ->
    if static
    then "static method in "^scope
    else "method in "^scope
  | SS.ClassVar (static, scope) ->
    if static
    then "static class var in "^scope
    else "class var in "^scope
  | SS.Function -> "function"
  | SS.Typedef -> "typedef"
  | SS.Constant -> "constant"
  | SS.Namespace -> "namespace"

let result_to_json res =
  let desc_string = desc_string_from_type res.SUtils.result_type in
  let scope_string = scope_string_from_type res.SUtils.result_type in
  let p = res.SUtils.pos in
  let fn = Pos.filename p in
  let line, start, end_ = Pos.info_pos p in
  Hh_json.JSON_Object
    [ "name", Hh_json.JSON_String (Utils.strip_ns res.SUtils.name);
      "filename",  Hh_json.JSON_String fn;
      "desc",  Hh_json.JSON_String desc_string;
      "line",  Hh_json.int_ line;
      "char_start", Hh_json.int_ start;
      "char_end", Hh_json.int_ end_;
      "scope", Hh_json.JSON_String scope_string;
    ]

let re_colon_colon = Str.regexp "::"

let go workers query type_ (env: SearchUtils.local_tracking_env)
  : SearchUtils.result =
  let max_results = 100 in
  let start_time = Unix.gettimeofday () in
  let fuzzy = SymbolIndex.fuzzy_search_enabled () in
  let results =
    (* If query contains "::", search class methods instead of top level definitions *)
    match Str.split_delim re_colon_colon query with
    | [class_name_query; method_query] ->
      (* Get the class with the most similar name to `class_name_query` *)
      let class_ =

        (* Switch between old behavior and new *)
        match SymbolIndex.get_search_provider () with
        | SearchUtils.TrieIndex ->
          SymbolIndex.query_for_symbol_search ~fuzzy workers class_name_query type_
          |> List.find ~f:begin fun result ->
            match result with
            | SearchUtils.{result_type = SearchUtils.Class _; _} -> true
            | _ -> false
          end
          |> Option.map ~f:(fun s -> s.SearchUtils.name)
        | _ ->
          SymbolIndex.find_matching_symbols
            ~query_text:class_name_query
            ~max_results:1
            ~kind_filter:(Some SearchUtils.SI_Class)
            ~context:None
            ~env
          |> List.hd
          |> Option.map ~f:(fun r -> r.SearchUtils.si_name)
      in
      begin match class_ with
      | Some name ->
        ClassMethodSearch.query_class_methods name method_query
      | None ->
        (* When we can't find a class with a name similar to the given one,
           just return no search results. *)
        []
      end
    | _  ->

      (* Switch between old behavior and new *)
      match SymbolIndex.get_search_provider () with
      | SearchUtils.TrieIndex ->
        let temp_results = SymbolIndex.query_for_symbol_search ~fuzzy workers query type_ in
        List.map temp_results SearchUtils.to_absolute
      | _ ->
        let temp_results = SymbolIndex.find_matching_symbols
          ~query_text:query
          ~max_results
          ~kind_filter:None
          ~context:None
          ~env
        in
        AutocompleteService.add_position_to_results temp_results
  in
  SymbolIndex.log_symbol_index_search
    ~start_time
    ~query_text:query
    ~max_results
    ~kind_filter:None
    ~results:(List.length results)
    ~context:None
    ~caller:"ServerSearch.go";
  results
