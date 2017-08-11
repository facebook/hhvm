(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module PositionedSyntax = Full_fidelity_positioned_syntax
module FFUtils = Full_fidelity_syntax_utilities.WithSyntax(PositionedSyntax)
open FfpAutocompleteContextParser
open AutocompleteTypes
open Core

let should_complete_class (context:context) : bool =
  let open ContextPredicates in
  let open Container in
  let open Predecessor in
  is_expression_valid context
  ||
  is_type_valid context
  ||
  (context.closest_parent_container = CompoundStatement ||
  context.closest_parent_container = AssignmentExpression) &&
  context.predecessor = KeywordNew
  ||
  (context.closest_parent_container = ClassHeader ||
  context.closest_parent_container = ClassBody) &&
  context.predecessor = KeywordExtends

let should_complete_interface (context:context) : bool =
  let open ContextPredicates in
  let open Container in
  let open Predecessor in
  is_expression_valid context
  ||
  is_type_valid context
  ||
  (context.closest_parent_container = InterfaceHeader ||
  context.closest_parent_container = InterfaceBody) &&
  context.predecessor = KeywordExtends
  ||
  (context.closest_parent_container = ClassHeader ||
  context.closest_parent_container = ClassBody) &&
  context.predecessor = KeywordImplements

let should_complete_trait (context:context) : bool =
  let open ContextPredicates in
  let open Container in
  let open Predecessor in
  is_expression_valid context
  ||
  (context.closest_parent_container = ClassBody &&
  context.inside_class_body &&
  context.predecessor = KeywordUse)

(* TODO: Port over the namespace completion code from autocompleteService.ml *)
let should_get_globals (context:context) : bool =
  should_complete_class context ||
  should_complete_interface context ||
  should_complete_trait context

let make_class_completion (context:context) (name:string) =
  if should_complete_class context then
    Some ({
      res_pos = Pos.none |> Pos.to_absolute;
      res_ty = "class";
      res_name = name;
      res_kind = Class_kind;
      func_details = None;
    })
  else
    None

let make_interface_completion (context:context) (name:string) =
  if should_complete_interface context then
    Some ({
      res_pos = Pos.none |> Pos.to_absolute;
      res_ty = "interface";
      res_name = name;
      res_kind = Interface_kind;
      func_details = None;
    })
  else
    None

let make_trait_completion (context:context) (name:string) =
  if should_complete_trait context then
    Some ({
      res_pos = Pos.none |> Pos.to_absolute;
      res_ty = "trait";
      res_name = name;
      res_kind = Trait_kind;
      func_details = None;
    })
  else
    None

let get_same_file_definitions (positioned_tree:PositionedSyntax.t)
  : string list * string list * string list =
  let open PositionedSyntax in
  let open PositionedToken in
  let open TokenKind in
  let aux (classes, interfaces, traits) node = match syntax node with
  | ClassishDeclaration { classish_keyword = {
      syntax = Token { kind = Class; _ }; _
    }; classish_name; _ } ->
    (PositionedSyntax.text classish_name)::classes, interfaces, traits
  | ClassishDeclaration { classish_keyword = {
      syntax = Token { kind = Interface; _ }; _
    }; classish_name; _ } ->
    classes, (PositionedSyntax.text classish_name)::interfaces, traits
  | ClassishDeclaration { classish_keyword = {
      syntax = Token { kind = Trait; _ }; _
    }; classish_name; _ } ->
    classes, interfaces, (PositionedSyntax.text classish_name)::traits
  | _ -> (classes, interfaces, traits)
  in
  FFUtils.fold aux ([], [], []) positioned_tree

let get_globals (context:context) (input:string) (positioned_tree:PositionedSyntax.t)
  : complete_autocomplete_result list =
  if should_get_globals context then
    let open Utils.With_complete_flag in
    let (classes, interfaces, traits) = get_same_file_definitions positioned_tree in
    let completions = List.concat_no_order [
      List.filter_map ~f:(make_class_completion context) classes;
      List.filter_map ~f:(make_trait_completion context) traits;
      List.filter_map ~f:(make_interface_completion context) interfaces;
    ]
    in
    let {value; _} =
      HackSearchService.MasterApi.query_autocomplete input ~limit:(Some 100)
        ~filter_map:begin fun _ _ res ->
          let name = Utils.strip_ns res.SearchUtils.name in
          match res.SearchUtils.result_type with
          | HackSearchService.Class (Some Ast.Ctrait) -> make_trait_completion context name
          | HackSearchService.Class (Some Ast.Cinterface) -> make_interface_completion context name
          | HackSearchService.Class _ -> make_class_completion context name
          | _ -> None
        end
    in
    completions @ value
  else
    []
