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
 * Multiple fields packed into one 64-bit integer, e.g. :
 *
 *    6         5         4         3         2         1         0
 * 3210987654321098765432109876543210987654321098765432109876543210
 * <----------------------------><----------------------><------->X
 *             field1                     field2           field3
 *
 * Fields are variants of Field.t, and the number of bits for each field
 * are given by Field.nbits.
 *)
type t = int [@@deriving eq, ord]

module Field = struct
  type t =
    | Start_beginning_of_line
        (** The character number of the beginning of line of the start position of the span. *)
    | Start_line_number
        (** The line number of the start position of the span. *)
    | Start_column_number
        (** The column number of the start position of the span. *)
    | Beginning_of_line_increment
        (** The beginning of line character number of the end position of the span
            is given by adding this increment to the beginning of line character number of the start position. *)
    | Line_number_increment
        (** The line number of the end position of the span
            is given by adding this increment to the line number of the start position. *)
    | Column_number_increment
        (** The column number of the end position of the span
            is given by adding this increment to the column number of the start position. *)
  [@@deriving enum, ord]

  let min_field : int = min

  let max_field : int = max

  (* These numbers were obtained by gathering statistics on the positions in the decl heap
   * for a large code base run as per december 2020. They should allow to encode about 99% of positions. *)
  (* /!\ Always make sure the total is 63, due to OCaml reserving 1 bit. *)
  let nbits : t -> int = function
    | Start_beginning_of_line -> 21
    | Start_line_number -> 16
    | Start_column_number -> 20
    | Beginning_of_line_increment -> 0
    | Line_number_increment -> 0
    | Column_number_increment -> 6

  let left_most =
    match of_enum min_field with
    | None -> assert false
    | Some field -> field

  let right_most =
    match of_enum max_field with
    | None -> assert false
    | Some field -> field

  let lhs_field : t -> t option =
   fun field ->
    let field_nb = to_enum field in
    of_enum (field_nb - 1)

  let rhs_field : t -> t option =
   fun field ->
    let field_nb = to_enum field in
    of_enum (field_nb + 1)

  let n_rhs_bits : t -> int =
   fun field ->
    let rec n_rhs_bits field ~acc =
      match rhs_field field with
      | None -> acc
      | Some prev_field ->
        let acc = acc + nbits prev_field in
        n_rhs_bits prev_field ~acc
    in
    n_rhs_bits field ~acc:0

  let dummy_value : t -> int = function
    | Start_beginning_of_line
    | Start_line_number
    | Column_number_increment
    | Beginning_of_line_increment
    | Line_number_increment ->
      0
    | Start_column_number -> -1
end

module FieldMap = WrappedMap.Make (Field)
module F = Field

type field = Field.t

type mask = int

let mask_of_size : int -> mask = (fun n -> (1 lsl n) - 1)

let dummy = -1

let is_dummy t = t = dummy

let project : field -> t -> int =
 fun field span ->
  if is_dummy span then
    F.dummy_value field
  else
    (span lsr F.n_rhs_bits field) land mask_of_size (F.nbits field)

let start_line_number = project F.Start_line_number

let line_number_increment = project F.Line_number_increment

let end_line_number span = start_line_number span + line_number_increment span

let start_beginning_of_line = project F.Start_beginning_of_line

let beginning_of_line_increment = project F.Beginning_of_line_increment

let end_beginning_of_line span =
  start_beginning_of_line span + beginning_of_line_increment span

let start_column = project F.Start_column_number

let column_increment = project F.Column_number_increment

let end_column span = start_column span + column_increment span

let start_character_number span =
  start_beginning_of_line span + start_column span

let end_character_number span = end_beginning_of_line span + end_column span

(** Project a field on a large span. *)
let project_large :
    pos_start:File_pos_large.t -> pos_end:File_pos_large.t -> field -> int =
 fun ~pos_start:
       {
         File_pos_large.pos_lnum = start_line_number;
         pos_bol = start_beginning_of_line;
         pos_cnum = start_character_number;
       }
     ~pos_end:
       {
         File_pos_large.pos_lnum = end_line_number;
         pos_bol = end_beginning_of_line;
         pos_cnum = end_character_number;
       }
     field ->
  let start_column_number = start_character_number - start_beginning_of_line in
  let end_column_number = end_character_number - end_beginning_of_line in
  match field with
  | F.Start_beginning_of_line -> start_beginning_of_line
  | F.Start_line_number -> start_line_number
  | F.Start_column_number -> start_column_number
  | F.Beginning_of_line_increment ->
    end_beginning_of_line - start_beginning_of_line
  | F.Line_number_increment -> end_line_number - start_line_number
  | F.Column_number_increment -> end_column_number - start_column_number

let make : pos_start:File_pos_large.t -> pos_end:File_pos_large.t -> t option =
 fun ~pos_start ~pos_end ->
  let value_of = project_large ~pos_start ~pos_end in
  let rec set_field_and_proceed_right field span =
    match field with
    | None -> Some span
    | Some field ->
      let field_value = value_of field in
      if Int.equal field_value (field_value land mask_of_size (F.nbits field))
      then
        let span = span lsl F.nbits field in
        let span = span lor field_value in
        set_field_and_proceed_right (F.rhs_field field) span
      else
        None
  in
  set_field_and_proceed_right (Some Field.left_most) 0

let to_field_map : t -> int FieldMap.t =
 fun span ->
  let rec to_list span field map_acc =
    match field with
    | None -> map_acc
    | Some field ->
      let value = span land mask_of_size (F.nbits field) in
      let map_acc = FieldMap.add field value map_acc in
      let span = span lsr F.nbits field in
      let field = F.lhs_field field in
      to_list span field map_acc
  in
  to_list span (Some Field.right_most) FieldMap.empty

let as_large_span : t -> File_pos_large.t * File_pos_large.t =
 fun span ->
  if is_dummy span then
    (File_pos_large.dummy, File_pos_large.dummy)
  else
    let field_map = to_field_map span in
    let start_line_number = FieldMap.find F.Start_line_number field_map in
    let start_beginning_of_line =
      FieldMap.find F.Start_beginning_of_line field_map
    in
    let start_column_number = FieldMap.find F.Start_column_number field_map in
    let pos_start =
      {
        File_pos_large.pos_lnum = start_line_number;
        pos_bol = start_beginning_of_line;
        pos_cnum = start_beginning_of_line + start_column_number;
      }
    in
    let end_line_number =
      start_line_number + FieldMap.find F.Line_number_increment field_map
    in
    let end_beginning_of_line =
      start_beginning_of_line
      + FieldMap.find F.Beginning_of_line_increment field_map
    in
    let end_column_number =
      start_column_number + FieldMap.find F.Column_number_increment field_map
    in
    let pos_end =
      {
        File_pos_large.pos_lnum = end_line_number;
        pos_bol = end_beginning_of_line;
        pos_cnum = end_beginning_of_line + end_column_number;
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

let show : t -> string =
 fun p ->
  let buffer = Buffer.create 255 in
  pp (Format.formatter_of_buffer buffer) p;
  Buffer.contents buffer
