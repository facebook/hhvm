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

(* An atom is a substring of the original source which will be exactly
 * represented in the formatted output and is considered indivisible by hackfmt.
 *
 * An atom is one of the following:
 * - A single token
 * - A single-line comment or delimited comment containing no newlines
 * - A segment of a multiline delimited comment. Multiline delimited comments
 *   are broken on newlines and stripped of their indentation whitespace so that
 *   their segments can be reindented properly.
 * - A single instance of ExtraTokenError trivia
 *
 * We keep track of the atoms that make up a chunk to make it possible
 * to format ranges which begin or end inside chunks (essential for as-you-type
 * formatting, where the cursor may be in the middle of a chunk).
 *
 * This data structure associates an atom's offset in the original source text
 * with its offset in a chunk. *)
type atom = {
  atom_text: string;
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
  atoms: atom list;
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
  atoms = [];
}

let make rule nesting start_char =
  let c = match rule with
    | None -> default_chunk
    | Some rule -> {default_chunk with rule}
  in
  {c with start_char; nesting}

let add_atom c atom_text width source_offset =
  let chunk_offset = String.length c.text in
  let atoms = {atom_text; width; source_offset; chunk_offset} :: c.atoms in
  {c with atoms; text = c.text ^ atom_text}

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
    atoms = List.rev chunk.atoms;
  }

let get_nesting_id chunk =
  chunk.nesting.Nesting.id

let get_range chunk =
  (chunk.start_char, chunk.end_char)

let print_range chunk range =
  let source_end atom = atom.source_offset + atom.width in
  let chunk_end atom = atom.chunk_offset + atom.width in
  let range_start, range_end = range in
  (* Find the nearest atom which starts at or before range_start, then get the
   * offset of that atom's first character in the chunk text *)
  let start_chunk_offset =
    chunk.atoms
    |> List.rev
    |> List.find ~f:(fun atom -> atom.source_offset <= range_start)
    |> Option.value_map ~default:0 ~f:(fun atom -> atom.chunk_offset)
  in
  (* Find the nearest atom which ends at or after range_end, then get the
   * offset (+ 1) of the atom's last character in the chunk text *)
  let end_chunk_offset =
    chunk.atoms
    |> List.find ~f:(fun atom -> range_end <= source_end atom)
    |> Option.value_map ~default:(String.length chunk.text) ~f:chunk_end
  in
  let width = end_chunk_offset - start_chunk_offset in
  String.sub chunk.text start_chunk_offset width
