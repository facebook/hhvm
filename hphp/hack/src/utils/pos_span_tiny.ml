(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** A compressed representation of a position span, i.e. a start and an end position. *)

(**
 * Pos_span_tiny.t packs multiple fields into one 63-bit integer:
 *
 *    6         5         4         3         2         1         0
 * 3210987654321098765432109876543210987654321098765432109876543210
 * <-------------------><--------------><------------------><---->X
 *  byte offset of line   line number      column number     width
 *)
type t = int [@@deriving eq, hash, ord]

let start_beginning_of_line_bits = 21

let start_line_number_bits = 16

let start_column_number_bits = 20

let beginning_of_line_increment_bits = 0

let line_number_increment_bits = 0

let width_bits = 6

(* The offset of each field (i.e., the number of bits to the right of it) is
 * the offset of the field to the right plus that field's bit width. *)

let width_offset = 0

let line_number_increment_offset = width_offset + width_bits

let beginning_of_line_increment_offset =
  line_number_increment_offset + line_number_increment_bits

let start_column_number_offset =
  beginning_of_line_increment_offset + beginning_of_line_increment_bits

let start_line_number_offset =
  start_column_number_offset + start_column_number_bits

let start_beginning_of_line_offset =
  start_line_number_offset + start_line_number_bits

(* The total number of bits used must be 63 (OCaml reserves one bit). *)
let () =
  assert (63 = start_beginning_of_line_bits + start_beginning_of_line_offset)

let mask bits = (1 lsl bits) - 1

let mask_by ~bits x = x land mask bits

let max_start_beginning_of_line = mask start_beginning_of_line_bits

let max_start_line_number = mask start_line_number_bits

let max_start_column_number = mask start_column_number_bits

let max_beginning_of_line_increment = mask beginning_of_line_increment_bits

let max_line_number_increment = mask line_number_increment_bits

let max_width = mask width_bits

let dummy = -1

let is_dummy t = t = dummy

let start_beginning_of_line (span : t) =
  if is_dummy span then
    0
  else
    mask_by
      ~bits:start_beginning_of_line_bits
      (span lsr start_beginning_of_line_offset)

let start_line_number (span : t) =
  if is_dummy span then
    0
  else
    mask_by ~bits:start_line_number_bits (span lsr start_line_number_offset)

let start_column (span : t) =
  if is_dummy span then
    -1
  else
    mask_by ~bits:start_column_number_bits (span lsr start_column_number_offset)

let beginning_of_line_increment (span : t) =
  if is_dummy span then
    0
  else
    mask_by
      ~bits:beginning_of_line_increment_bits
      (span lsr beginning_of_line_increment_offset)

let line_number_increment (span : t) =
  if is_dummy span then
    0
  else
    mask_by
      ~bits:line_number_increment_bits
      (span lsr line_number_increment_offset)

let width (span : t) =
  if is_dummy span then
    0
  else
    mask_by ~bits:width_bits (span lsr width_offset)

let start_offset span = start_beginning_of_line span + start_column span

let end_line_number span = start_line_number span + line_number_increment span

let end_beginning_of_line span =
  start_beginning_of_line span + beginning_of_line_increment span

let end_offset span = start_offset span + width span

let end_column span = end_offset span - end_beginning_of_line span

let make : pos_start:File_pos_large.t -> pos_end:File_pos_large.t -> t option =
 fun ~pos_start ~pos_end ->
  if File_pos_large.is_dummy pos_start || File_pos_large.is_dummy pos_end then
    Some dummy
  else
    let {
      File_pos_large.pos_lnum = start_line;
      pos_bol = start_bol;
      pos_offset = start_offset;
    } =
      pos_start
    in
    let {
      File_pos_large.pos_lnum = end_line;
      pos_bol = end_bol;
      pos_offset = end_offset;
    } =
      pos_end
    in
    if
      start_offset < start_bol
      || end_bol < start_bol
      || end_line < start_line
      || end_offset < start_offset
    then
      None
    else
      let start_col = start_offset - start_bol in
      let bol_increment = end_bol - start_bol in
      let line_increment = end_line - start_line in
      let width = end_offset - start_offset in
      if
        start_bol > max_start_beginning_of_line
        || start_line > max_start_line_number
        || start_col > max_start_column_number
        || bol_increment > max_beginning_of_line_increment
        || line_increment > max_line_number_increment
        || width > max_width
      then
        None
      else
        Some
          ((start_bol lsl start_beginning_of_line_offset)
          lor (start_line lsl start_line_number_offset)
          lor (start_col lsl start_column_number_offset)
          lor (bol_increment lsl beginning_of_line_increment_offset)
          lor (line_increment lsl line_number_increment_offset)
          lor (width lsl width_offset))

let as_large_span : t -> File_pos_large.t * File_pos_large.t =
 fun span ->
  if is_dummy span then
    (File_pos_large.dummy, File_pos_large.dummy)
  else
    let start_lnum = start_line_number span in
    let start_bol = start_beginning_of_line span in
    let start_offset = start_offset span in
    let end_lnum = end_line_number span in
    let end_bol = end_beginning_of_line span in
    let end_offset = end_offset span in
    let pos_start =
      {
        File_pos_large.pos_lnum = start_lnum;
        pos_bol = start_bol;
        pos_offset = start_offset;
      }
    in
    let pos_end =
      {
        File_pos_large.pos_lnum = end_lnum;
        pos_bol = end_bol;
        pos_offset = end_offset;
      }
    in
    (pos_start, pos_end)

let pp fmt span =
  Format.pp_print_int fmt (start_line_number span);
  Format.pp_print_string fmt ":";
  Format.pp_print_int fmt (start_column span + 1);
  Format.pp_print_string fmt "-";
  if start_line_number span = end_line_number span then
    Format.pp_print_int fmt @@ (end_column span + 1)
  else (
    Format.pp_print_int fmt (end_line_number span);
    Format.pp_print_string fmt ":";
    Format.pp_print_int fmt (end_column span + 1)
  )

let show pos = Format.asprintf "%a" pp pos
