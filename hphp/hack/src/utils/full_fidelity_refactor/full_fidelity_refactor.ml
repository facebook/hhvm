(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_editable_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module Trivia = Full_fidelity_positioned_trivia
module TriviaKind = Full_fidelity_trivia_kind
module RT = ServerRenameTypes

let find_width_hh_fixme token =
  let rec find_width_hh_fixme_rec acc last sd =
    match sd with
    | sde :: sd ->
      (match Trivia.kind sde with
      | TriviaKind.FixMe
      | TriviaKind.IgnoreError ->
        let new_width = Trivia.width sde + acc in
        find_width_hh_fixme_rec new_width new_width sd
      | TriviaKind.SingleLineComment
      | TriviaKind.DelimitedComment ->
        last
      | _ -> find_width_hh_fixme_rec (Trivia.width sde + acc) last sd)
    | [] -> last
  in
  match token.Syn.Token.token_data with
  | Syn.Token.Original sd ->
    find_width_hh_fixme_rec 0 0 (List.rev sd.Syn.SourceData.leading)
  | _ -> 0

let insert_before_leading_fixme file ~keyword ~text =
  match Syn.syntax keyword with
  | Syn.Token token ->
    Syn.Value.(
      let value = from_token token in
      (match value with
      | Positioned source_data ->
        let source_text = Syn.SourceData.source_text source_data in
        let start_offset = Syn.SourceData.start_offset source_data in
        let end_offset = Syn.SourceData.end_offset source_data in
        let leading_text_width = find_width_hh_fixme token in
        let pos =
          Syn.SourceText.relative_pos
            file
            source_text
            (start_offset - leading_text_width)
            end_offset
        in
        Some RT.(Insert { pos = Pos.to_absolute pos; text })
      | Synthetic -> None))
  | _ -> None

let insert_attribute file ~attribute ~enclosing_node ~attributes_node =
  let open Syn in
  match syntax attributes_node with
  | OldAttributeSpecification { old_attribute_specification_attributes; _ } ->
    (* if other attributes are already present, add to that list *)
    let attributes =
      syntax_node_to_list old_attribute_specification_attributes
    in
    if
      List.exists
        ~f:(fun node ->
          match syntax node with
          | ListItem { list_item; _ } -> String.equal (text list_item) attribute
          | _ -> false)
        attributes
    then
      None
    else
      Option.Monad_infix.(
        position_exclusive file old_attribute_specification_attributes
        >>| fun pos ->
        RT.(Insert { pos = Pos.to_absolute pos; text = attribute ^ ", " }))
  | _ ->
    (* there are no other attributes, but we must distinguish
       * if there is a leading string (eg HHFIXME) or not *)
    let default_patch inline node =
      Option.Monad_infix.(
        position_exclusive file node >>| fun pos ->
        RT.(
          Insert
            {
              pos = Pos.to_absolute pos;
              text =
                (if inline then
                  " <<" ^ attribute ^ ">> "
                else
                  "<<" ^ attribute ^ ">>\n");
            }))
    in
    let insert_attribute_before_leading_fixme keyword =
      Option.first_some
        (insert_before_leading_fixme
           file
           ~keyword
           ~text:("<<" ^ attribute ^ ">>\n"))
        (default_patch false keyword)
    in

    (match enclosing_node with
    | Some enclosing_node ->
      (* if there is a leading string (eg. HHFIXME) and no other attributes,
          * put the attribute before the leading string *)
      (match syntax enclosing_node with
      | FunctionDeclaration { function_declaration_header; _ } ->
        (match syntax function_declaration_header with
        | FunctionDeclarationHeader { function_modifiers; function_keyword; _ }
          ->
          (match Syntax.kind function_modifiers with
          | SyntaxKind.Missing ->
            (* there are no modifiers, so analyise the "function" keyword *)
            insert_attribute_before_leading_fixme function_keyword
          | _ ->
            let modifiers = syntax_node_to_list function_modifiers in
            insert_attribute_before_leading_fixme (List.hd_exn modifiers))
        | _ -> default_patch false function_declaration_header)
      | ClassishDeclaration
          { classish_modifiers; classish_xhp; classish_keyword; _ } ->
        (match Syntax.kind classish_modifiers with
        | SyntaxKind.Missing ->
          (match Syntax.kind classish_xhp with
          | SyntaxKind.Missing ->
            (* there are no modifiers, so analyise the "class" keyword *)
            insert_attribute_before_leading_fixme classish_keyword
          | _ -> insert_attribute_before_leading_fixme classish_xhp)
        | _ ->
          let modifiers = syntax_node_to_list classish_modifiers in
          insert_attribute_before_leading_fixme (List.hd_exn modifiers))
      | _ -> default_patch false enclosing_node)
    | None -> default_patch true attributes_node)
