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
 * - An empty string representing a blank line. When this atom occurs, it must
 *   be the only atom in the chunk.
 *
 * We keep track of the atoms that make up a chunk to make it possible
 * to format ranges which begin or end inside chunks (essential for as-you-type
 * formatting, where the cursor may be in the middle of a chunk).
 *
 * This data structure associates atoms with their location in the original
 * source. *)
type atom = {
  text: string;
  range: Interval.t;
  leading_space: bool;
}

type t = {
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
  length: int;
}

let default_chunk = {
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
  length = -1;
}

let make rule nesting start_char =
  let c = match rule with
    | None -> default_chunk
    | Some rule -> {default_chunk with rule}
  in
  {c with start_char; nesting}

let add_atom c ?(leading_space=false) text width source_offset =
  let range = source_offset, source_offset + width in
  {c with atoms = {text; range; leading_space} :: c.atoms}

let finalize chunk rule ra space comma end_char =
  let end_char = max chunk.start_char end_char in
  let rule = if Rule_allocator.get_rule_kind ra rule = Rule.Always
    || chunk.rule = Rule.null_rule_id
    then rule
    else chunk.rule
  in
  let atom_length atom =
    (if atom.leading_space then 1 else 0) + String.length atom.text
  in
  let length =
    chunk.atoms
    |> List.map ~f:atom_length
    |> List.fold ~init:0 ~f:(+)
  in
  {chunk with
    is_appendable = false;
    rule;
    space_if_not_split = space;
    comma_rule = comma;
    end_char;
    atoms = List.rev chunk.atoms;
    length;
  }

let get_nesting_id chunk =
  chunk.nesting.Nesting.id

let get_range chunk =
  (chunk.start_char, chunk.end_char)

let is_empty chunk =
  match chunk.atoms with
  | [] -> true
  | [atom] when atom.text = "" -> true
  | _ -> false

let text chunk =
  let buf = Buffer.create chunk.length in
  List.iter chunk.atoms ~f:begin fun atom ->
    if atom.leading_space then Buffer.add_char buf ' ';
    Buffer.add_string buf atom.text;
  end;
  Buffer.contents buf
