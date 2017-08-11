(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open FfpAutocompleteContextParser
open AutocompleteTypes

let should_complete_function (context:context) : bool =
  let open ContextPredicates in
  is_expression_valid context

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
  should_complete_function context ||
  should_complete_interface context ||
  should_complete_trait context

let make_class_completion (name:string) (context:context) =
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

let make_interface_completion (name:string) (context:context) =
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

let make_trait_completion (name:string) (context:context) =
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

let make_function_completion (name:string) (context:context) =
  if should_complete_function context then
    Some ({
      res_pos = Pos.none |> Pos.to_absolute;
      res_ty = "function";
      res_name = name;
      res_kind = Function_kind;
      func_details = None;
    })
  else
    None

let get_globals (context:context) (input:string) : complete_autocomplete_result list =
  if should_get_globals context then
    let open Utils.With_complete_flag in
    let {value; _} =
      HackSearchService.MasterApi.query_autocomplete input ~limit:(Some 100)
        ~filter_map:begin fun _ _ res ->
          let name = Utils.strip_ns res.SearchUtils.name in
          match res.SearchUtils.result_type with
          | HackSearchService.Class (Some Ast.Ctrait) -> make_trait_completion name context
          | HackSearchService.Class (Some Ast.Cinterface) -> make_interface_completion name context
          | HackSearchService.Class _ -> make_class_completion name context
          | HackSearchService.Function -> make_function_completion name context
          | _ -> None
        end
    in
    value
  else
    []
