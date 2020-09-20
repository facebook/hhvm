(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Format_env
open Hh_prelude

(* A subchunk is one of these variants, representing a substring of the final
 * string representation of a chunk in the formatted output.
 *
 * This representation is a convenient intermediate step because it allows us to
 * trim whitespace and trailing commas at the edges of formatted ranges as
 * needed.
 *)
type t =
  | Atom of {
      text: string;
      range: Interval.t;
    }
  | Comma
  | Space
  | Newline
  | Indent of int

let string_of_subchunks (env : Env.t) (subchunks : t list) : string =
  let buf = Buffer.create 200 in
  List.iter subchunks ~f:(function
      | Atom { text; _ } -> Buffer.add_string buf text
      | Comma -> Buffer.add_char buf ','
      | Space -> Buffer.add_char buf ' '
      | Newline -> Buffer.add_char buf '\n'
      | Indent indent ->
        Buffer.add_string buf
        @@
        if env.Env.indent_with_tabs then
          String.make indent '\t'
        else
          String.make (indent * env.Env.indent_width) ' ');
  Buffer.contents buf

let subchunks_in_range
    ~(include_leading_whitespace : bool)
    ~(include_trailing_whitespace : bool)
    (subchunks : t list)
    (range : Interval.t) : t list =
  let (range_start, range_end) = range in
  (* Filter to the atoms which overlap with the range, including the leading and
   * trailing non-Atom subchunks. *)
  let subchunks =
    List.take_while subchunks ~f:(function
        | Atom { range = (st, ed); _ } ->
          st < range_end || (st = range_end && ed = range_end)
        | _ -> true)
  in
  let subchunks = List.rev subchunks in
  let subchunks =
    List.take_while subchunks ~f:(function
        | Atom { range = (st, ed); _ } ->
          ed > range_start || (st = range_start && ed = range_start)
        | _ -> true)
  in
  (* Drop trailing spaces and indentation *)
  let subchunks =
    List.drop_while subchunks ~f:(function
        | Space
        | Indent _ ->
          true
        | _ -> false)
  in
  (* When omitting trailing whitespace, drop trailing newlines *)
  let subchunks =
    if include_trailing_whitespace then
      subchunks
    else
      List.drop_while subchunks ~f:(function
          | Newline -> true
          | _ -> false)
  in
  let subchunks = List.rev subchunks in
  (* Drop leading newline. Also drop leading Commas. Comma represents a
   * trailing comma that was added because its associated split was broken on.
   * Since we don't have source mapping information for Commas (since there may
   * have been no trailing comma in the original source), we only want to print
   * them when the atom preceding the trailing comma is printed. If a Comma is
   * at the beginning of the range, then the atom preceding it was not
   * included, and neither should the Comma. *)
  let subchunks =
    List.drop_while subchunks ~f:(function
        | Newline
        | Space
        | Comma ->
          true
        | _ -> false)
  in
  (* When omitting leading whitespace, drop leading indentation *)
  let subchunks =
    if include_leading_whitespace then
      subchunks
    else
      List.drop_while subchunks ~f:(function
          | Indent _ -> true
          | _ -> false)
  in
  subchunks

let subchunks_of_solve_state (state : Solve_state.t) : t list =
  let subchunks = ref [] in
  let add_subchunk sc = subchunks := sc :: !subchunks in
  List.iter (Solve_state.chunks state) ~f:(fun chunk ->
      if Solve_state.has_split_before_chunk state ~chunk then (
        add_subchunk Newline;
        if (not (Chunk.is_empty chunk)) && chunk.Chunk.indentable then
          add_subchunk (Indent (Solve_state.get_indent_level state chunk))
      ) else if chunk.Chunk.space_if_not_split then
        add_subchunk Space;

      List.iter chunk.Chunk.atoms ~f:(fun atom ->
          let { Chunk.leading_space; text; range } = atom in
          if leading_space then add_subchunk Space;
          add_subchunk (Atom { text; range }));

      if Solve_state.has_comma_after_chunk state ~chunk then
        match Chunk.get_comma_range chunk with
        | Some range -> add_subchunk (Atom { text = ","; range })
        | None -> add_subchunk Comma);

  (* Every chunk group has a newline trailing it, but chunks are associated with
   * the split preceding them. In order to ensure that we print trailing
   * newlines and not leading newlines, we always add a newline here, and strip
   * leading newlines below. *)
  add_subchunk Newline;
  let subchunks = List.rev !subchunks in
  let subchunks =
    List.drop_while subchunks ~f:(function
        | Newline -> true
        | _ -> false)
  in
  subchunks

let debug (subchunks : t list) : string =
  let buf = Buffer.create 200 in
  List.iter subchunks ~f:(function
      | Atom { text; range = (st, ed) } ->
        Buffer.add_string buf (Printf.sprintf "Atom %d,%d %S\n" st ed text)
      | Comma -> Buffer.add_string buf "Comma\n"
      | Space -> Buffer.add_string buf "Space\n"
      | Newline -> Buffer.add_string buf "Newline\n"
      | Indent indent ->
        Buffer.add_string buf (Printf.sprintf "Indent %d\n" indent));
  Buffer.contents buf
