(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * An EditablePositionedSyntax represents a syntax that comes from a positioned
 * source but may have been modified. The syntax may have had its children
 * changed, or may have been moved to a new location in the AST. An
 * EditablePositionedSyntax falls into one of these categories:
 *
 *   - Positioned.  The syntax was positioned in the original source text. It or
 *     its children may have been modified.
 *   - Synthetic.  The syntax never existed in the original SourceText. It was
 *     synthesized during the AST computation process (colloquially,
 *     "lowering"). Note that this does not imply that all of its children are
 *     synthetic.
 *)

open Hh_prelude
module Token = Full_fidelity_editable_positioned_token
module PositionedSyntax = Full_fidelity_positioned_syntax

module SourceData = Full_fidelity_editable_positioned_original_source_data

module SourceText = Full_fidelity_source_text

module Value = struct
  type t =
    | Positioned of SourceData.t
    | Synthetic
  [@@deriving show, eq]

  let from_positioned_syntax syntax =
    Positioned (SourceData.from_positioned_syntax syntax)

  let from_token token =
    match token.Token.token_data with
    | Token.Original original_source_data
    | Token.SynthesizedFromOriginal (_, original_source_data) ->
      Positioned original_source_data
    | Token.Synthetic _ -> Synthetic

  let to_json value =
    Hh_json.(
      SourceData.(
        match value with
        | Positioned { offset; leading_width; width; trailing_width; _ } ->
          JSON_Object
            [
              ("offset", int_ offset);
              ("leading_width", int_ leading_width);
              ("width", int_ width);
              ("trailing_width", int_ trailing_width);
            ]
        | Synthetic -> JSON_String "synthetic"))
end

module SyntaxWithToken = Full_fidelity_syntax.WithToken (Token)
module Syntax = SyntaxWithToken.WithSyntaxValue (Value)
include Syntax

(**
 * Recursively reconstructs a PositionedSyntax as a hierarchy of
 * EditablePositionedSyntaxes, each of which is Positioned.
 *)
let rec from_positioned_syntax node =
  let syntax =
    match PositionedSyntax.syntax node with
    | PositionedSyntax.Token token -> Token (Token.from_positioned_token token)
    | _ ->
      node
      |> PositionedSyntax.children
      |> List.map ~f:from_positioned_syntax
      |> syntax_from_children (PositionedSyntax.kind node)
  in
  make syntax (Value.from_positioned_syntax node)

let synthesize_from editable_positioned_syntax syntax =
  { editable_positioned_syntax with syntax }

(**
 * Computes the text from constituent tokens.
 *)
let text node =
  match all_tokens node with
  | [] -> ""
  | [hd] -> Token.text hd
  | hd :: tl ->
    (match List.rev tl with
    | [] -> assert false
    | last :: interior_tokens_rev ->
      let interior_full_text =
        interior_tokens_rev
        |> List.rev
        |> List.map ~f:Token.full_text
        |> String.concat ~sep:""
      in
      Token.text hd
      ^ Token.trailing_text hd
      ^ interior_full_text
      ^ Token.leading_text last
      ^ Token.text last)

let leading_text node =
  Option.value_map ~default:"" ~f:Token.leading_text (leading_token node)

let trailing_text node =
  Option.value_map ~default:"" ~f:Token.trailing_text (trailing_token node)

let full_text node =
  node |> all_tokens |> List.map ~f:Token.full_text |> String.concat ~sep:""

let original_source_data_or_default node =
  match value node with
  | Value.Positioned source_data -> source_data
  | Value.Synthetic -> SourceData.empty

let source_text node =
  SourceData.source_text (original_source_data_or_default node)

let leading_width node =
  SourceData.leading_width (original_source_data_or_default node)

let width node = SourceData.width (original_source_data_or_default node)

let trailing_width node =
  SourceData.trailing_width (original_source_data_or_default node)

let full_width node =
  SourceData.full_width (original_source_data_or_default node)

let leading_start_offset node =
  SourceData.leading_start_offset (original_source_data_or_default node)

let start_offset node =
  SourceData.start_offset (original_source_data_or_default node)

let end_offset node =
  SourceData.end_offset (original_source_data_or_default node)

let trailing_start_offset node =
  leading_start_offset node + leading_width node + width node

let offset node =
  match value node with
  | Value.Positioned source_data -> Some (SourceData.start_offset source_data)
  | Value.Synthetic -> None

let position file node =
  match value node with
  | Value.Positioned source_data ->
    let source_text = SourceData.source_text source_data in
    let start_offset = SourceData.start_offset source_data in
    let end_offset = SourceData.end_offset source_data in
    Some (SourceText.relative_pos file source_text start_offset end_offset)
  | Value.Synthetic -> None

let position_exclusive file node =
  match value node with
  | Value.Positioned source_data ->
    let source_text = SourceData.source_text source_data in
    let start_offset = SourceData.start_offset source_data in
    let end_offset = SourceData.end_offset source_data + 1 in
    Some (SourceText.relative_pos file source_text start_offset end_offset)
  | Value.Synthetic -> None

let extract_text node = Some (text node)

module ValueBuilder = struct
  open Value

  let value_from_children _source _offset _kind = function
    | [] ->
      (* Missing node case: we consider Missing to be Synthetic. *)
      Synthetic
    | hd :: tl ->
      (match (value hd, Option.map ~f:value (List.last tl)) with
      | (_, None) ->
        (* Single node case: use that node's value. *)
        value hd
      | (Positioned b, Some (Positioned e)) ->
        (* First and last child are positioned: Reconstruct source data. *)
        Positioned (SourceData.spanning_between b e)
      | (_, Some _) ->
        (* Otherwise: Not enough information to position this node. *)
        Synthetic)

  let value_from_token token = from_token token

  let value_from_syntax syntax =
    let pr first last =
      match (first, last) with
      | (Value.Synthetic, Value.Synthetic) -> Synthetic
      | (f, Value.Synthetic) -> f
      | (Positioned f, Positioned l) ->
        Positioned (SourceData.spanning_between f l)
      | (_, _) -> Synthetic
    in
    let folder (sum : Value.t * Value.t) child : Value.t * Value.t =
      match sum with
      | (Value.Synthetic, Value.Synthetic) -> (value child, Value.Synthetic)
      | (f, _) -> (f, value child)
    in
    let (first, last) =
      Syntax.fold_over_children folder (Value.Synthetic, Value.Synthetic) syntax
    in
    pr first last
end

(* TODO: This code is duplicated in the positioned syntax; consider pulling it
out into its own module. *)
(* Takes a node and an offset; produces the descent through the parse tree
   to that position. *)
let parentage node position =
  let rec aux nodes position acc =
    match nodes with
    | [] -> acc
    | h :: t ->
      let width = full_width h in
      if position < width then
        aux (children h) position (h :: acc)
      else
        aux t (position - width) acc
  in
  aux [node] position []

let is_in_body node position =
  let rec aux parents =
    match parents with
    | [] -> false
    | h1 :: h2 :: _
      when is_compound_statement h1
           && (is_methodish_declaration h2 || is_function_declaration h2) ->
      true
    | _ :: rest -> aux rest
  in
  let parents = parentage node position in
  aux parents

let is_synthetic node =
  match value node with
  | Value.Synthetic -> true
  | Value.Positioned _ -> false

include Syntax.WithValueBuilder (ValueBuilder)

let rust_parse _ _ = failwith "not implemented"

let rust_parse_with_verify_sc _ _ = failwith "not implemented"

let rust_parser_errors _ _ _ = failwith "not implemented"
