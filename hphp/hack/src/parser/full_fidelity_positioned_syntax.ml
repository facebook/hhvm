(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
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

open Core_kernel
module SourceText = Full_fidelity_source_text
module Token = Full_fidelity_positioned_token
module SyntaxWithPositionedToken = Full_fidelity_syntax.WithToken (Token)

module PositionedSyntaxValue = struct
  type t =
    (* value for a token node is token itself *)
    | TokenValue of Token.t
    (* value for a range denoted by pair of tokens *)
    | TokenSpan of {
        left: Token.t;
        right: Token.t;
      }
    | Missing of {
        source_text: SourceText.t;
        offset: int;
      }
  [@@deriving show]

  let source_text value =
    match value with
    | TokenValue t
    | TokenSpan { left = t; _ } ->
      Token.source_text t
    | Missing { source_text; _ } -> source_text

  let start_offset value =
    match value with
    | TokenValue t
    | TokenSpan { left = t; _ } ->
      Token.leading_start_offset t
    | Missing { offset; _ } -> offset

  let leading_width value =
    match value with
    | TokenValue t
    | TokenSpan { left = t; _ } ->
      Token.leading_width t
    | Missing _ -> 0

  let width value =
    match value with
    | TokenValue t -> Token.width t
    | TokenSpan { left; right } ->
      Token.end_offset right - Token.start_offset left + 1
    | Missing _ -> 0

  let trailing_width value =
    match value with
    | TokenValue t
    | TokenSpan { right = t; _ } ->
      Token.trailing_width t
    | Missing _ -> 0

  let to_json value =
    Hh_json.(
      JSON_Object
        [
          ("offset", int_ (start_offset value));
          ("leading_width", int_ (leading_width value));
          ("width", int_ (width value));
          ("trailing_width", int_ (trailing_width value));
        ])
end

module PositionedWithValue =
  SyntaxWithPositionedToken.WithSyntaxValue (PositionedSyntaxValue)
include PositionedWithValue

module PositionedValueBuilder = struct
  let value_from_token token =
    if
      Token.kind token = Full_fidelity_token_kind.EndOfFile
      || Token.full_width token = 0
    then
      PositionedSyntaxValue.Missing
        {
          source_text = Token.source_text token;
          offset = Token.end_offset token;
        }
    else
      PositionedSyntaxValue.TokenValue token

  let value_from_outer_children first last =
    let module PSV = PositionedSyntaxValue in
    match (value first, value last) with
    | (PSV.TokenValue l, PSV.TokenValue r)
    | (PSV.TokenValue l, PSV.TokenSpan { right = r; _ })
    | (PSV.TokenSpan { left = l; _ }, PSV.TokenValue r)
    | (PSV.TokenSpan { left = l; _ }, PSV.TokenSpan { right = r; _ }) ->
      if phys_equal l r then
        PSV.TokenValue l
      else
        PSV.TokenSpan { left = l; right = r }
    (* can have two missing nodes if first and last child nodes of
       the node are missing - this means that entire node is missing.
       NOTE: offset must match otherwise it will mean that there is a real node
       in between that should be picked instead *)
    | (PSV.Missing { offset = o1; source_text }, PSV.Missing { offset = o2; _ })
      when o1 = o2 ->
      PSV.Missing { offset = o1; source_text }
    | _ -> assert false

  let width n = PositionedSyntaxValue.width (value n)

  let value_from_children source_text offset _kind nodes =
    (*
     * We need to determine the offset, leading, middle and trailing widths of
     * the node to be constructed based on its children.  If the children are
     * all of zero width -- including the case where there are no children at
     * all -- then we make a zero-width value at the given offset.
     * Otherwise, we can determine the associated value from the first and last
     * children that have width.
     *)
    let have_width = List.filter ~f:(fun x -> width x > 0) nodes in
    match have_width with
    | [] -> PositionedSyntaxValue.Missing { source_text; offset }
    | first :: _ -> value_from_outer_children first (List.last_exn have_width)

  let value_from_syntax syntax =
    let module PSV = PositionedSyntaxValue in
    (* We need to find the first and last nodes that are represented by tokens.
      If there are no such nodes then we can simply use the first and last nodes, period,
      since they will have an offset and source text we can use. *)
    let f (first, first_not_zero, last_not_zero, _last) node =
      match (first, first_not_zero, value node) with
      | (None, None, (PSV.TokenValue _ | PSV.TokenSpan _)) ->
        (* first iteration and first node has some token representation -
           record it as first, first_non_zero, last and last_non_zero *)
        (Some node, Some node, Some node, Some node)
      | (None, None, _) ->
        (* first iteration - first node is missing -
          record it as first and last *)
        (Some node, None, None, Some node)
      | (Some _, None, (PSV.TokenValue _ | PSV.TokenSpan _)) ->
        (* in progress, found first node that include tokens -
          record it as first_non_zero, last and last_non_zero  *)
        (first, Some node, Some node, Some node)
      | (Some _, Some _, (PSV.TokenValue _ | PSV.TokenSpan _)) ->
        (* in progress found some node that include tokens -
          record it as last_non_zero and last *)
        (first, first_not_zero, Some node, Some node)
      | _ ->
        (* in progress, stepped on missing node -
           record it as last and move on *)
        (first, first_not_zero, last_not_zero, Some node)
    in
    let (f, fnz, lnz, l) =
      fold_over_children f (None, None, None, None) syntax
    in
    match (f, fnz, lnz, l) with
    | (_, Some first_not_zero, Some last_not_zero, _) ->
      value_from_outer_children first_not_zero last_not_zero
    | (Some first, None, None, Some last) ->
      value_from_outer_children first last
    | _ ->
      failwith "how did we get a node with no children in value_from_syntax?"
end

include PositionedWithValue.WithValueBuilder (PositionedValueBuilder)
module Validated =
  Full_fidelity_validated_syntax.Make (Token) (PositionedSyntaxValue)

let source_text node = PositionedSyntaxValue.source_text (value node)

let leading_width node = PositionedSyntaxValue.leading_width (value node)

let width node = PositionedSyntaxValue.width (value node)

let trailing_width node = PositionedSyntaxValue.trailing_width (value node)

let full_width node = leading_width node + width node + trailing_width node

let leading_start_offset node = PositionedSyntaxValue.start_offset (value node)

let leading_end_offset node =
  let w = leading_width node - 1 in
  let w =
    if w < 0 then
      0
    else
      w
  in
  leading_start_offset node + w

let start_offset node = leading_start_offset node + leading_width node

let end_offset node =
  let w = width node - 1 in
  let w =
    if w < 0 then
      0
    else
      w
  in
  start_offset node + w

let trailing_start_offset node =
  leading_start_offset node + leading_width node + width node

let trailing_end_offset node =
  let w = full_width node - 1 in
  let w =
    if w < 0 then
      0
    else
      w
  in
  leading_start_offset node + w

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

let leading_span node = (leading_start_position node, leading_end_position node)

let span node = (start_position node, end_position node)

let trailing_span node =
  (trailing_start_position node, trailing_end_position node)

let full_span node = (leading_start_position node, trailing_end_position node)

let full_text node =
  SourceText.sub
    (source_text node)
    (leading_start_offset node)
    (full_width node)

let leading_text node =
  SourceText.sub
    (source_text node)
    (leading_start_offset node)
    (leading_width node)

let trailing_text node =
  SourceText.sub (source_text node) (end_offset node + 1) (trailing_width node)

let text node =
  SourceText.sub (source_text node) (start_offset node) (width node)

let trailing_token node =
  match value node with
  | PositionedSyntaxValue.TokenValue token -> Some token
  | PositionedSyntaxValue.TokenSpan { right; _ } -> Some right
  | PositionedSyntaxValue.Missing _ -> None

let extract_text node = Some (text node)

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

let is_synthetic _node = false

let leading_trivia node =
  let token = leading_token node in
  match token with
  | None -> []
  | Some t -> Token.leading t

let trailing_trivia node =
  let token = trailing_token node in
  match token with
  | None -> []
  | Some t -> Token.trailing t

type 'a rust_parse_type =
  Full_fidelity_source_text.t ->
  Full_fidelity_parser_env.t ->
  'a * t * Full_fidelity_syntax_error.t list * Rust_pointer.t option

let rust_parse_ref : unit rust_parse_type ref =
  ref (fun _ _ -> failwith "This should be lazily set in Rust_parser_ffi")

let rust_parse text env = !rust_parse_ref text env

let rust_parse_with_coroutine_sc_ref : bool rust_parse_type ref =
  ref (fun _ _ -> failwith "This should be lazily set in Rust_parser_ffi")

let rust_parse_with_coroutine_sc text env =
  !rust_parse_with_coroutine_sc_ref text env

let rust_parse_with_decl_mode_sc_ref : bool list rust_parse_type ref =
  ref (fun _ _ -> failwith "This should be lazily set in Rust_parser_ffi")

let rust_parse_with_decl_mode_sc text env =
  !rust_parse_with_decl_mode_sc_ref text env

let rust_parse_with_verify_sc_ref : t list rust_parse_type ref =
  ref (fun _ _ -> failwith "This should be lazily set in Rust_parser_ffi")

let rust_parse_with_verify_sc text env = !rust_parse_with_verify_sc_ref text env

external rust_parser_errors :
  Full_fidelity_source_text.t ->
  Rust_pointer.t ->
  ParserOptions.ffi_t ->
  Full_fidelity_syntax_error.t list = "rust_parser_errors_positioned"
