(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Editable syntax tree
 *
 *  Every token and trivia in the tree knows its text. Therefore we can add
 *  new nodes without having to back them with a source text.
 *)

module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)
module EditableToken = Full_fidelity_editable_token
module PositionedSyntax = Full_fidelity_positioned_syntax
module SyntaxWithEditableToken = Full_fidelity_syntax.WithToken(EditableToken)

(**
 * Ironically, an editable syntax tree needs even less per-node information
 * than the "positioned" syntax tree, which needs to know the width of the node.
 **)

module EditableSyntaxValue = struct
  type t = NoValue
  let to_json value =
    let open Hh_json in
    JSON_Object [ ]
end

module EditableSyntax =
  SyntaxWithEditableToken.WithSyntaxValue(EditableSyntaxValue)

module EditableValueBuilder = struct
  let value_from_children _ _ _ _ =
    EditableSyntaxValue.NoValue

  let value_from_token _ =
    EditableSyntaxValue.NoValue

  let value_from_syntax _ =
    EditableSyntaxValue.NoValue
end

include EditableSyntax
include EditableSyntax.WithValueBuilder(EditableValueBuilder)

let rec from_positioned text positioned_node offset =
  match PositionedSyntax.syntax positioned_node with
  | PositionedSyntax.Token token ->
    let editable_token = EditableToken.from_positioned text token offset in
    let syntax = Token editable_token in
    make syntax EditableSyntaxValue.NoValue
  | _ ->
    let folder (acc, offset) child =
      let new_child = from_positioned text child offset in
      let w = PositionedSyntax.full_width child in
      (new_child :: acc, offset + w) in
    let kind = PositionedSyntax.kind positioned_node in
    let positioneds = PositionedSyntax.children positioned_node in
    let (editables, _) = List.fold_left folder ([], offset) positioneds in
    let editables = List.rev editables in
    let syntax = syntax_from_children kind editables in
    make syntax EditableSyntaxValue.NoValue

let from_tree tree =
  from_positioned (SyntaxTree.text tree) (SyntaxTree.root tree) 0

let text node =
  let buffer = Buffer.create 100 in
  let aux token =
    Buffer.add_string buffer (EditableToken.full_text token) in
  List.iter aux (all_tokens node);
  Buffer.contents buffer

(* Takes a node and an offset; produces the descent through the parse tree
   to that position. *)
let parentage root position =
  let rec aux nodes position acc =
    match nodes with
    | [] -> acc
    | h :: t ->
      let width = String.length @@ text h in
      if position < width then
        aux (children h) position (h :: acc)
      else
        aux t (position - width) acc in
  aux [root] position []

let leading_trivia node =
  let token = leading_token node in
  match token with
  | None -> []
  | Some t -> EditableToken.leading t

let trailing_trivia node =
  let token = trailing_token node in
  match token with
  | None -> []
  | Some t -> EditableToken.trailing t

let to_json node =
  let version = Full_fidelity_schema.full_fidelity_schema_version_number in
  let tree = EditableSyntax.to_json node in
  Hh_json.JSON_Object [
    "parse_tree", tree;
    "version", Hh_json.JSON_String version
  ]
