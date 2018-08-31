(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Typed and positioned syntax tree
 *
 * A positioned tree which may know information about the type of the node.
 *
 * Types come from the typed ast.
 * type annotations are available on all:
 *   - expressions
 *   - class-id
 *     - instanceof
 *     - new
 *     - get
 *     - const
 * TODO: Enable type annotations on:
 *   - tast hints (aka type annotations):
 *     - hint -> ty: Decl_hint.hint, Decl_env
 *     - parameters
 *     - return types
 *     - explicit type args on call
 *     - cast
 *     - is
 *     - as
 *     - tparam constraints
 *     - c_extends- uses
 *     - req-extends
 *     - req-implements
 *     - implements
 *     - class constant
 *     - property
 *     - type
 *     - constant
 *)

open Core_kernel
module SourceText = Full_fidelity_positioned_syntax.SourceText
module Token = Full_fidelity_positioned_token
module Trivia = Full_fidelity_positioned_trivia
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SyntaxWithToken = Full_fidelity_syntax.WithToken(Token)


(*
  Converts from full fidelity positions (which use a different indexing scheme
  from typed ast positions) to json where:
  - all indexes are 0 based
  - start columns are inclusive
  - end columns are exclusive
*)
let pos_to_zero_indexed_json t =
  let line_start, char_start, line_end, char_end = Pos.destruct_range t in
  let fn = Pos.filename t in
  Hh_json.JSON_Object [
    "filename", Hh_json.JSON_String fn;
    "start",    Hh_json.JSON_Object [
        "line",   Hh_json.int_ (line_start - 1);
        "column", Hh_json.int_ (char_start - 1)
    ];
    "end",      Hh_json.JSON_Object [
        "line",   Hh_json.int_ (line_end - 1);
        "column", Hh_json.int_ char_end
    ];
  ]

module Value = struct
  (* placeholder definitions *)
  type absolute = Pos.absolute
  let show_absolute _ = "<absolute>"
  let pp_absolute _ _ = Printf.printf "%s\n" "<asbolute>"

  type t = {
    source_text: SourceText.t;
    offset: int; (* Beginning of first trivia *)
    leading_width: int;
    width: int; (* Width of node, not counting trivia *)
    trailing_width: int;
    tys: Tast_type_collector.collected_type list;
    position: absolute;
  } [@@deriving show]

  let position value =
    SourceText.relative_pos
      (SourceText.file_path value.source_text)
      value.source_text value.offset
      (value.offset + value.width)

  let to_json value =
    let open Hh_json in
    JSON_Object
      (["position", pos_to_zero_indexed_json value.position
      ] @ match value.tys with
      | [] -> []
      | tys ->
        let json_tys = Tast_type_collector.collected_types_to_json tys in
        [("types", JSON_Array json_tys)])
end

module PositionedSyntaxValue = Full_fidelity_positioned_syntax.PositionedSyntaxValue

let positioned_value_to_typed
  (position: Pos.absolute)
  (types: Tast_type_collector.collected_type list)
  (value: PositionedSyntaxValue.t): Value.t =
  {
    Value.source_text = PositionedSyntaxValue.source_text value;
    offset = PositionedSyntaxValue.start_offset value;
    leading_width = PositionedSyntaxValue.leading_width value;
    width = PositionedSyntaxValue.width value;
    trailing_width = PositionedSyntaxValue.trailing_width value;
    tys = types;
    position;
  }

module TypedSyntax = SyntaxWithToken.WithSyntaxValue(Value)

module TypedValueBuilder = struct
  let value_from_children _ _ _ _ =
    assert false

  let value_from_token _ =
    assert false

  let value_from_syntax _ =
    assert false
end

include TypedSyntax
include TypedSyntax.WithValueBuilder(TypedValueBuilder)


let source_text node =
  (value node).Value.source_text

let leading_width node =
  (value node).Value.leading_width

let width node =
  (value node).Value.width

let trailing_width node =
  (value node).Value.trailing_width

let full_width node =
  (leading_width node) + (width node) + (trailing_width node)

let leading_start_offset node =
  (value node).Value.offset

let leading_end_offset node =
  let w = (leading_width node) - 1 in
  let w = if w < 0 then 0 else w in
  (leading_start_offset node) + w

let start_offset node =
  (leading_start_offset node) + (leading_width node)

let end_offset node =
  let w = (width node) - 1 in
  let w = if w < 0 then 0 else w in
  (start_offset node) + w

let trailing_start_offset node =
  (leading_start_offset node) + (leading_width node) + (width node)

let trailing_end_offset node =
  let w = (full_width node) - 1 in
  let w = if w < 0 then 0 else w in
  (leading_start_offset node) + w

let leading_start_position node =
  SourceText.offset_to_position (source_text node) (leading_start_offset node)

let leading_end_position node =
  SourceText.offset_to_position (source_text node) (leading_end_offset node)

let start_position node =
  SourceText.offset_to_position (source_text node) (start_offset node)

let end_position node =
  SourceText.offset_to_position (source_text node) (end_offset node)

let trailing_start_position node =
  SourceText.offset_to_position (source_text node) (trailing_start_offset node)

let trailing_end_position node =
  SourceText.offset_to_position (source_text node) (trailing_end_offset node)

let leading_span node =
  ((leading_start_position node), (leading_end_position node))

let span node =
  ((start_position node), (end_position node))

let trailing_span node =
  ((trailing_start_position node), (trailing_end_position node))

let full_span node =
  ((leading_start_position node), (trailing_end_position node))

let full_text node =
  SourceText.sub
    (source_text node) (leading_start_offset node) (full_width node)

let leading_text node =
  SourceText.sub
    (source_text node)
    (leading_start_offset node)
    (leading_width node)

let trailing_text node =
  SourceText.sub
    (source_text node) ((end_offset node) + 1) (trailing_width node)

let text node =
  SourceText.sub (source_text node) (start_offset node) (width node)

let extract_text node =
  Some (text node)

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
        aux t (position - width) acc in
  aux [node] position []

let is_in_body node position =
  let rec aux parents =
    match parents with
    | [] -> false
    | h1 :: t1 ->
      if is_compound_statement h1 then
        match t1 with
        | [] -> false
        | h2 :: _ ->
          is_methodish_declaration h2 || is_function_declaration h2 || aux t1
      else
        aux t1 in
  let parents = parentage node position in
  aux parents

let position (_file: Relative_path.t) (node: t) =
  Some (Value.position node.value)

let offset node = Some (start_offset node)
