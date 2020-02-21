(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module PositionedSyntax = Full_fidelity_positioned_syntax
open FfpAutocompleteContextParser
open AutocompleteTypes

let should_complete_class (context : context) : bool =
  let open ContextPredicates in
  let open Container in
  let open Predecessor in
  is_expression_valid context
  || is_type_valid context
  || ( context.closest_parent_container = CompoundStatement
     || context.closest_parent_container = AssignmentExpression )
     && context.predecessor = KeywordNew
  || ( context.closest_parent_container = ClassHeader
     || context.closest_parent_container = ClassBody )
     && context.predecessor = KeywordExtends

let should_complete_interface (context : context) : bool =
  let open ContextPredicates in
  let open Container in
  let open Predecessor in
  is_expression_valid context
  || is_type_valid context
  || ( context.closest_parent_container = InterfaceHeader
     || context.closest_parent_container = InterfaceBody )
     && context.predecessor = KeywordExtends
  || ( context.closest_parent_container = ClassHeader
     || context.closest_parent_container = ClassBody )
     && context.predecessor = KeywordImplements

let should_complete_trait (context : context) : bool =
  let open ContextPredicates in
  let open Container in
  let open Predecessor in
  is_expression_valid context
  || context.closest_parent_container = ClassBody
     && context.inside_class_body
     && context.predecessor = KeywordUse

(* TODO: Port over the namespace completion code from autocompleteService.ml *)
let should_get_globals (context : context) : bool =
  should_complete_class context
  || should_complete_interface context
  || should_complete_trait context

let make_class_completion
    (context : context) (replace_pos : Ide_api_types.range) (name : string) =
  if should_complete_class context then
    Some
      {
        res_pos = Pos.none |> Pos.to_absolute;
        res_replace_pos = replace_pos;
        res_base_class = None;
        res_ty = "class";
        res_name = name;
        res_fullname = name;
        res_kind = SearchUtils.SI_Class;
        func_details = None;
        ranking_details = None;
      }
  else
    None

let make_interface_completion
    (context : context) (replace_pos : Ide_api_types.range) (name : string) =
  if should_complete_interface context then
    Some
      {
        res_pos = Pos.none |> Pos.to_absolute;
        res_replace_pos = replace_pos;
        res_base_class = None;
        res_ty = "interface";
        res_name = name;
        res_fullname = name;
        res_kind = SearchUtils.SI_Interface;
        func_details = None;
        ranking_details = None;
      }
  else
    None

let make_trait_completion
    (context : context) (replace_pos : Ide_api_types.range) (name : string) =
  if should_complete_trait context then
    Some
      {
        res_pos = Pos.none |> Pos.to_absolute;
        res_replace_pos = replace_pos;
        res_base_class = None;
        res_ty = "trait";
        res_name = name;
        res_fullname = name;
        res_kind = SearchUtils.SI_Trait;
        func_details = None;
        ranking_details = None;
      }
  else
    None

let get_same_file_definitions (positioned_tree : PositionedSyntax.t) :
    string list * string list * string list =
  let open PositionedSyntax in
  let open PositionedToken in
  let open TokenKind in
  let aux (classes, interfaces, traits) node =
    match syntax node with
    | ClassishDeclaration
        {
          classish_keyword = { syntax = Token { kind = Class; _ }; _ };
          classish_name;
          _;
        } ->
      (PositionedSyntax.text classish_name :: classes, interfaces, traits)
    | ClassishDeclaration
        {
          classish_keyword = { syntax = Token { kind = Interface; _ }; _ };
          classish_name;
          _;
        } ->
      (classes, PositionedSyntax.text classish_name :: interfaces, traits)
    | ClassishDeclaration
        {
          classish_keyword = { syntax = Token { kind = Trait; _ }; _ };
          classish_name;
          _;
        } ->
      (classes, interfaces, PositionedSyntax.text classish_name :: traits)
    | _ -> (classes, interfaces, traits)
  in
  let rec fold acc node =
    let acc = aux acc node in
    List.fold_left ~f:fold ~init:acc (PositionedSyntax.children node)
  in
  fold ([], [], []) positioned_tree

let get_globals
    (sienv : SearchUtils.si_env)
    (context : context)
    (query_text : string)
    (positioned_tree : PositionedSyntax.t)
    (replace_pos : Ide_api_types.range) : complete_autocomplete_result list =
  if should_get_globals context then
    let (classes, interfaces, traits) =
      get_same_file_definitions positioned_tree
    in
    let completions =
      List.concat_no_order
        [
          List.filter_map ~f:(make_class_completion context replace_pos) classes;
          List.filter_map ~f:(make_trait_completion context replace_pos) traits;
          List.filter_map
            ~f:(make_interface_completion context replace_pos)
            interfaces;
        ]
    in
    let results =
      SymbolIndex.find_matching_symbols
        ~sienv
        ~query_text
        ~max_results:100
        ~context:None
        ~kind_filter:None
    in
    let value =
      List.filter_map results ~f:(fun res ->
          let name = Utils.strip_ns res.SearchUtils.si_fullname in
          match res.SearchUtils.si_kind with
          | SearchUtils.SI_Trait ->
            make_trait_completion context replace_pos name
          | SearchUtils.SI_Interface ->
            make_interface_completion context replace_pos name
          | SearchUtils.SI_Enum
          | SearchUtils.SI_Class ->
            make_class_completion context replace_pos name
          | _ -> None)
    in
    completions @ value
  else
    []
