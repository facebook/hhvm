(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

let find_function_at_pos
    (positioned_tree : PositionedTree.t) ~(function_pos : Pos.t) :
    Full_fidelity_positioned_syntax.t option =
  let root = PositionedTree.root positioned_tree in
  let (start_offset, _end_offset) = Pos.info_raw function_pos in
  let nodes = Full_fidelity_positioned_syntax.parentage root start_offset in
  List.find_map nodes ~f:(fun node ->
      match node.Full_fidelity_positioned_syntax.syntax with
      | Full_fidelity_positioned_syntax.MethodishDeclaration _
      | Full_fidelity_positioned_syntax.FunctionDeclaration _ ->
        Some node
      | _ -> None)

let get_attribute_insert_info
    (function_node : Full_fidelity_positioned_syntax.t) : int option * bool =
  let get_insert_info_from_attribute_spec attribute_spec =
    match attribute_spec.Full_fidelity_positioned_syntax.syntax with
    | Full_fidelity_positioned_syntax.Missing ->
      let offset = Full_fidelity_positioned_syntax.start_offset function_node in
      (Some offset, false)
    | Full_fidelity_positioned_syntax.OldAttributeSpecification
        { old_attribute_specification_attributes; _ } -> begin
      match
        old_attribute_specification_attributes
          .Full_fidelity_positioned_syntax.syntax
      with
      | Full_fidelity_positioned_syntax.SyntaxList attrs -> begin
        match List.last attrs with
        | Some last_attr ->
          (* end_offset points to the last character; +1 to insert after it *)
          let offset =
            Full_fidelity_positioned_syntax.end_offset last_attr + 1
          in
          (Some offset, true)
        | None ->
          (* Empty attribute list `<<>>` doesn't parse, so this is unreachable *)
          (None, false)
      end
      | _ -> (None, false)
    end
    | _ -> (None, false)
  in
  match function_node.Full_fidelity_positioned_syntax.syntax with
  | Full_fidelity_positioned_syntax.MethodishDeclaration
      { methodish_attribute; _ } ->
    get_insert_info_from_attribute_spec methodish_attribute
  | Full_fidelity_positioned_syntax.FunctionDeclaration
      { function_attribute_spec; _ } ->
    get_insert_info_from_attribute_spec function_attribute_spec
  | _ -> (None, false)

let create_attribute_text
    ~(attribute_name : string) ~(has_existing_attrs : bool) ~(indent : string) :
    string =
  if has_existing_attrs then
    ", " ^ attribute_name
  else
    "<<" ^ attribute_name ^ ">>\n" ^ indent

let create
    (ctx : Provider_context.t)
    (entry : Provider_context.entry)
    ~(function_pos : Pos.t)
    ~(attribute_name : string) : Code_action_types.edit list =
  let positioned_tree = Ast_provider.compute_cst ~ctx ~entry in
  let path = entry.Provider_context.path in
  match entry.Provider_context.source_text with
  | None -> []
  | Some source_text ->
    (match find_function_at_pos positioned_tree ~function_pos with
    | None -> []
    | Some function_node ->
      (match get_attribute_insert_info function_node with
      | (Some offset, has_existing_attrs) ->
        let indent =
          if has_existing_attrs then
            ""
          else
            let (_line, col) =
              Full_fidelity_positioned_syntax.start_position function_node
            in
            (* col is 1-indexed, subtract 1 for 0-indexed indentation width *)
            String.make (col - 1) ' '
        in
        let text =
          create_attribute_text ~attribute_name ~has_existing_attrs ~indent
        in
        let pos =
          Full_fidelity_source_text.relative_pos path source_text offset offset
        in
        [Code_action_types.{ pos; text }]
      | (None, _) -> []))
