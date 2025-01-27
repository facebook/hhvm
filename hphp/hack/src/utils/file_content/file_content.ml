(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Ppx_yojson_conv_lib.Yojson_conv.Primitives

module Position : sig
  type t [@@deriving eq, ord, yojson_of]

  val from_one_based : int -> int -> t

  val from_zero_based : int -> int -> t

  val line_column_one_based : t -> int * int

  val line_column_zero_based : t -> int * int

  val beginning_of_file : t

  val beginning_of_line : t -> t

  val beginning_of_next_line : t -> t

  val is_beginning_of_line : t -> bool

  val next_char : t -> t

  val to_string_one_based : t -> string
end = struct
  type t = {
    line: int;  (** 1-based *)
    column: int;  (** 1-based *)
  }
  [@@deriving eq, ord, yojson_of]

  let first_line = 1

  let first_column = 1

  let beginning_of_file = { line = first_line; column = first_column }

  let from_one_based line column = { line; column }

  let from_zero_based line column = { line = line + 1; column = column + 1 }

  let line_column_one_based { line; column } = (line, column)

  let line_column_zero_based { line; column } = (line - 1, column - 1)

  let beginning_of_line { line; column = _ } = { line; column = first_column }

  let beginning_of_next_line { line; column = _ } =
    { line = line + 1; column = first_column }

  let is_beginning_of_line { line = _; column } = Int.equal column first_column

  let next_char { line; column } = { line; column = column + 1 }

  let to_string_one_based { line; column } = Printf.sprintf "%d:%d" line column
end

type range = {
  st: Position.t;
  ed: Position.t;
}

type text_edit = {
  range: range option;
  text: string;
}

(* UTF-8 encoding character lengths.
 *
 * NOTE: at the moment, edit commands are the only place where we count
 * UTF-8 encoded characters as opposed to ASCII bytes - in all of the other
 * places (column numbers in errors, positions in IDE commands) we still use the
 * latter.
 *
 * We make an exception here because that's the way Nuclide counts characters,
 * and the consequences of mishandling it are much more dire than in other
 * places - we'll not only fail the current single request, but diverge
 * the synchronized state forever.
 *)
let get_char_length c =
  let c = Char.to_int c in
  if c lsr 7 = 0b0 then
    1
  else if c lsr 5 = 0b110 then
    2
  else if c lsr 4 = 0b1110 then
    3
  else if c lsr 3 = 0b11110 then
    4
  else
    raise (Failure (Printf.sprintf "Invalid UTF-8 leading byte: %d" c))

let is_target t line column =
  Position.equal t (Position.from_one_based line column)

let get_char content offset =
  (* sentinel newline to make things easier *)
  if offset = String.length content then
    '\n'
  else
    content.[offset]

let rec get_offsets content queries line column offset acc =
  match acc with
  | (Some _, Some _) -> acc
  | (None, r2) when is_target (fst queries) line column ->
    get_offsets content queries line column offset (Some offset, r2)
  | ((Some _ as r1), None) when is_target (snd queries) line column ->
    get_offsets content queries line column offset (r1, Some offset)
  | acc ->
    let (line, column, offset) =
      match get_char content offset with
      | '\n' -> (line + 1, 1, offset + 1)
      | c -> (line, column + 1, offset + get_char_length c)
    in
    get_offsets content queries line column offset acc

let invalid_position p =
  raise
    (Failure
       (Printf.sprintf
          "Invalid position: %s"
          (Position.yojson_of_t p |> Yojson.Safe.to_string)))

(* this returns 0-based offsets *)
let get_offsets (content : string) (queries : Position.t * Position.t) :
    int * int =
  match get_offsets content queries 1 1 0 (None, None) with
  | (Some r1, Some r2) -> (r1, r2)
  | (None, _) -> invalid_position (fst queries)
  | (_, None) -> invalid_position (snd queries)

(* This returns a 0-based offset. If you need to get two offsets, use
   `get_offsets` instead. *)
let get_offset (content : string) (position : Position.t) : int =
  fst (get_offsets content (position, position))

(* This takes 0-based offsets and returns 1-based positions.                  *)
(* It gives the position of the character *immediately after* this offset,    *)
(* e.g. "offset_to_position s 0" gives the 1-based position {line=1,col=1}.   *)
(* It sounds confusing but is natural when you work with half-open ranges!    *)
(* It is okay to ask for the position of the offset of the end of the file.   *)
(* In case of multi-byte characters, if you give an offset inside a character,*)
(* it still gives the position immediately after.                             *)
let offset_to_position (content : string) (offset : int) : Position.t =
  let rec helper (p : Position.t) ~(index : int) =
    if index >= offset then
      p
    else
      let c = get_char content index in
      let clen = get_char_length c in
      if Char.equal c '\n' then
        helper (Position.beginning_of_next_line p) ~index:(index + clen)
      else
        helper (Position.next_char p) ~index:(index + clen)
  in
  if offset > String.length content then
    raise (Failure (Printf.sprintf "Invalid offset: %d" offset))
  else
    helper Position.beginning_of_file ~index:0

let apply_edit content { range; text } =
  match range with
  | None -> text
  | Some { st; ed } ->
    let (start_offset, end_offset) = get_offsets content (st, ed) in
    let prefix = Str.string_before content start_offset in
    let suffix = Str.string_after content end_offset in
    prefix ^ text ^ suffix

let print_edit b edit =
  let range =
    match edit.range with
    | None -> "None"
    | Some range ->
      Printf.sprintf
        "%s - %s"
        (Position.to_string_one_based range.st)
        (Position.to_string_one_based range.ed)
  in
  Printf.bprintf b "range = %s\n text = \n%s\n" range edit.text

let edit_file content (edits : text_edit list) :
    (string, string * Exception.t) result =
  try Ok (List.fold ~init:content ~f:apply_edit edits) with
  | exn ->
    let e = Exception.wrap exn in
    let b = Buffer.create 1024 in
    Printf.bprintf b "Invalid edit: %s\n" (Exception.get_ctor_string e);
    Printf.bprintf b "Original content:\n%s\n" content;
    Printf.bprintf b "Edits:\n";
    List.iter edits ~f:(print_edit b);
    Error (Buffer.contents b, e)

let edit_file_unsafe fc edits =
  match edit_file fc edits with
  | Ok r -> r
  | Error (e, _stack) ->
    Printf.eprintf "%s" e;
    failwith e
