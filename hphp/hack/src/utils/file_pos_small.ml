(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(**
 * Three values packed into one 64-bit integer:
 *
 *    6         5         4         3         2         1         0
 * 3210987654321098765432109876543210987654321098765432109876543210
 * <----------------------------><----------------------><------->X
 *       beginning of line                 line            column
 *
 * - (bol)  beginning of line (byte offset from start of file) starts at 0,
            maximum is 2^30-1 = 1,073,741,823
 * - (line) line number starts at 1, maximum is 2^24-1 = 16,777,215
 * - (col)  column number starts at 0, maximum is 2^9-1 = 511
 *            This is saturating, i.e. every column past 511 has column
 *            number 511 (so as not to raise exceptions).
 * - (X)    OCaml's tagged integer representation; 1 if int, 0 if pointer
 *
 *
 *)

type t = int [@@deriving eq, hash, ord]

let column_bits = 9

let line_bits = 24

let bol_bits = 30

let mask bits = (1 lsl bits) - 1

let mask_by ~bits x = x land mask bits

let max_column = mask column_bits

let max_line = mask line_bits

let max_bol = mask bol_bits

let dummy = -1

let is_dummy t = t = dummy

let beg_of_line (pos : t) =
  if is_dummy pos then
    0
  else
    mask_by ~bits:bol_bits (pos lsr (line_bits + column_bits))

let line (pos : t) =
  if is_dummy pos then
    0
  else
    mask_by ~bits:line_bits (pos lsr column_bits)

let column (pos : t) =
  if is_dummy pos then
    -1
  else
    mask_by ~bits:column_bits pos

let bol_line_col_unchecked bol line col =
  if col < 0 then
    dummy
  else
    (bol lsl (column_bits + line_bits)) + (line lsl column_bits) + col

let bol_line_col bol line col =
  if col > max_column || line > max_line || bol > max_bol then
    None
  else
    Some (bol_line_col_unchecked bol line col)

let pp fmt pos =
  Format.pp_print_int fmt (line pos);
  Format.pp_print_string fmt ":";
  Format.pp_print_int fmt (column pos + 1)

let compare : t -> t -> int = compare

let beg_of_file = bol_line_col_unchecked 0 1 0

(* constructors *)

let of_lexing_pos lp =
  bol_line_col
    lp.Lexing.pos_bol
    lp.Lexing.pos_lnum
    (lp.Lexing.pos_cnum - lp.Lexing.pos_bol)

let of_lnum_bol_offset ~pos_lnum ~pos_bol ~pos_offset =
  bol_line_col pos_bol pos_lnum (pos_offset - pos_bol)

(* accessors *)

let offset t = beg_of_line t + column t

let line_beg t = (line t, beg_of_line t)

let line_column t = (line t, column t)

let line_column_beg t = (line t, column t, beg_of_line t)

let line_column_offset t = (line t, column t, offset t)

let line_beg_offset t = (line t, beg_of_line t, offset t)

let set_column_unchecked c p = bol_line_col_unchecked (beg_of_line p) (line p) c

let set_column c p = bol_line_col (beg_of_line p) (line p) c

let to_lexing_pos pos_fname t =
  {
    Lexing.pos_fname;
    Lexing.pos_lnum = line t;
    Lexing.pos_bol = beg_of_line t;
    Lexing.pos_cnum = offset t;
  }

let as_large_pos p =
  let (lnum, col, bol) = line_column_beg p in
  File_pos_large.of_lnum_bol_offset
    ~pos_lnum:lnum
    ~pos_bol:bol
    ~pos_offset:(bol + col)

let of_large_pos : File_pos_large.t -> t option =
 fun { File_pos_large.pos_bol; pos_offset; pos_lnum } ->
  of_lnum_bol_offset ~pos_lnum ~pos_bol ~pos_offset
