(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 **
 *
 * Positioned syntax tree
 *
 * A positioned syntax tree stores the original source text,
 * the offset of the leading trivia, the width of the leading trivia,
 * node proper, and trailing trivia. From all this information we can
 * rapidly compute the absolute position of any portion of the node,
 * or the text.
 *
 *)

module SourceText = Full_fidelity_source_text
module Token = Full_fidelity_positioned_token

module SyntaxWithPositionedToken =
  Full_fidelity_syntax.WithToken(Token)

module PositionedSyntaxValue = struct
  type t = {
    source_text: SourceText.t;
    offset: int; (* Beginning of first trivia *)
    leading_width: int;
    width: int; (* Width of node, not counting trivia *)
    trailing_width: int;
  }

  let make source_text offset leading_width width trailing_width =
    { source_text; offset; leading_width; width; trailing_width }

  let source_text value =
    value.source_text

  let start_offset value =
    value.offset

  let leading_width value =
    value.leading_width

  let width value =
    value.width

  let trailing_width value =
    value.trailing_width

  let to_json value =
    let open Hh_json in
    JSON_Object
      [ "offset", int_ value.offset
      ; "leading_width", int_ value.leading_width
      ; "width", int_ value.width
      ; "trailing_width", int_ value.trailing_width
      ]
end


module PositionedWithValue =
  SyntaxWithPositionedToken.WithSyntaxValue(PositionedSyntaxValue)

open Hh_core
include PositionedWithValue

module PositionedValueBuilder = struct
  let value_from_token token =
    let source_text = Token.source_text token in
    let offset = Token.leading_start_offset token in
    let leading_width = Token.leading_width token in
    let width = Token.width token in
    let trailing_width = Token.trailing_width token in
    PositionedSyntaxValue.make
      source_text offset leading_width width trailing_width

  let value_from_outer_children first last =
    let first_value = value first in
    let last_value = value last in
    let source_text = PositionedSyntaxValue.source_text first_value in
    let first_offset = PositionedSyntaxValue.start_offset first_value in
    let first_leading_width = PositionedSyntaxValue.leading_width first_value in
    let trailing_width =
      PositionedSyntaxValue.trailing_width last_value in
    let last_offset = PositionedSyntaxValue.start_offset last_value in
    let last_leading_width = PositionedSyntaxValue.leading_width last_value in
    let last_width = PositionedSyntaxValue.width last_value in
    let width = (last_offset + last_leading_width + last_width) -
      (first_offset + first_leading_width) in
    PositionedSyntaxValue.make
      source_text first_offset first_leading_width width trailing_width

  let width n =
    PositionedSyntaxValue.width (value n)

  let value_from_children source_text offset kind nodes =
    (**
     * We need to determine the offset, leading, middle and trailing widths of
     * the node to be constructed based on its children.  If the children are
     * all of zero width -- including the case where there are no children at
     * all -- then we make a zero-width value at the given offset.
     * Otherwise, we can determine the associated value from the first and last
     * children that have width.
     *)
    let have_width = List.filter ~f:(fun x -> (width x) > 0) nodes in
    match have_width with
    | [] -> PositionedSyntaxValue.make source_text offset 0 0 0
    | first :: _ -> value_from_outer_children first (List.last_exn have_width)

  let value_from_syntax syntax =
    (* We need to find the first and last nodes that have width. If there are
      no such nodes then we can simply use the first and last nodes, period,
      since they will have an offset and source text we can use. *)
    let f (first, first_not_zero, last_not_zero, last) node =
      if first = None then
        if (width node) > 0 then
          (Some node, Some node, Some node, Some node)
        else
          (Some node, None, None, Some node)
      else if (width node) > 0 then
        if first_not_zero = None then
          (first, Some node, Some node, Some node)
        else
          (first, first_not_zero, Some node, Some node)
      else
        (first, first_not_zero, last_not_zero, Some node) in
    let (f, fnz, lnz, l) =
      fold_over_children f (None, None, None, None) syntax in
    match (f, fnz, lnz, l) with
    | (_, Some first_not_zero, Some last_not_zero, _) ->
      value_from_outer_children first_not_zero last_not_zero
    | (Some first, None, None, Some last) ->
      value_from_outer_children first last
    | _ -> failwith
      "how did we get a node with no children in value_from_syntax?"
end

include PositionedWithValue.WithValueBuilder(PositionedValueBuilder)

module Validated =
  Full_fidelity_validated_syntax.Make(Token)(PositionedSyntaxValue)

let source_text node =
  PositionedSyntaxValue.source_text (value node)

let leading_width node =
  PositionedSyntaxValue.leading_width (value node)

let width node =
  PositionedSyntaxValue.width (value node)

let trailing_width node =
  PositionedSyntaxValue.trailing_width (value node)

let full_width node =
  (leading_width node) + (width node) + (trailing_width node)

let leading_start_offset node =
  PositionedSyntaxValue.start_offset (value node)

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

let position file node =
  let source_text = source_text node in
  let start_offset = start_offset node in
  let end_offset = end_offset node in
  Some (SourceText.relative_pos file source_text start_offset end_offset)

let offset node = Some (start_offset node)

let position_exclusive file node =
  let source_text = source_text node in
  let start_offset = start_offset node in
  let end_offset = end_offset node + 1 in
  Some (SourceText.relative_pos file source_text start_offset end_offset)

let is_synthetic node = false

let leading_trivia node =
  let token = leading_token node in
  match token with
  | None -> []
  | Some t -> Token.leading t
