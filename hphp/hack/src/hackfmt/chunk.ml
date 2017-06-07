(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

(* Association between a token's offset in the original source text and its
 * offset in a chunk. *)
type token = {
  token_text: string;
  width: int;
  source_offset: int;
  chunk_offset: int;
}

type t = {
  text: string;
  spans: Span.t list;
  is_appendable: bool;
  space_if_not_split: bool;
  comma_rule: int option;
  rule: int;
  nesting: Nesting.t;
  start_char: int;
  end_char: int;
  indentable: bool;
  tokens: token list;
}

let default_chunk = {
  text = "";
  spans = [];
  is_appendable = true;
  space_if_not_split = false;
  comma_rule = None;
  rule = Rule.null_rule_id;
  nesting = Nesting.dummy;
  start_char = -1;
  end_char = -1;
  indentable = true;
  tokens = [];
}

let make rule nesting start_char =
  let c = match rule with
    | None -> default_chunk
    | Some rule -> {default_chunk with rule}
  in
  {c with start_char; nesting}

let add_token c token_text width source_offset =
  let chunk_offset = String.length c.text in
  let tokens = {token_text; width; source_offset; chunk_offset} :: c.tokens in
  {c with tokens; text = c.text ^ token_text}

let finalize chunk rule ra space comma end_char =
  let end_char = max chunk.start_char end_char in
  let rule = if Rule_allocator.get_rule_kind ra rule = Rule.Always
    || chunk.rule = Rule.null_rule_id
    then rule
    else chunk.rule
  in
  {chunk with
    is_appendable = false;
    rule;
    space_if_not_split = space;
    comma_rule = comma;
    end_char;
    tokens = List.rev chunk.tokens;
  }

let get_nesting_id chunk =
  chunk.nesting.Nesting.id

let get_range chunk =
  (chunk.start_char, chunk.end_char)

let print_range chunk range =
  let source_end tok = tok.source_offset + tok.width in
  let chunk_end tok = tok.chunk_offset + tok.width in
  let range_start, range_end = range in
  (* Find the nearest token which starts at or before range_start, then get the
   * offset of that token's first character in the chunk text *)
  let start_chunk_offset =
    chunk.tokens
    |> List.rev
    |> List.find ~f:(fun tok -> tok.source_offset <= range_start)
    |> Option.value_map ~default:0 ~f:(fun tok -> tok.chunk_offset)
  in
  (* Find the nearest token which ends at or after range_end, then get the
   * offset (+ 1) of the token's last character in the chunk text *)
  let end_chunk_offset =
    chunk.tokens
    |> List.find ~f:(fun tok -> range_end <= source_end tok)
    |> Option.value_map ~default:(String.length chunk.text) ~f:chunk_end
  in
  let width = end_chunk_offset - start_chunk_offset in
  String.sub chunk.text start_chunk_offset width
