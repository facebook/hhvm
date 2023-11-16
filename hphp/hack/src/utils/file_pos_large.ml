(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* See documentation in mli file *)
type t = {
  pos_lnum: int;
  pos_bol: int;
  pos_offset: int;
}
[@@deriving eq, hash, ord]

let pp fmt pos =
  Format.pp_print_int fmt pos.pos_lnum;
  Format.pp_print_string fmt ":";
  Format.pp_print_int fmt (pos.pos_offset - pos.pos_bol + 1)

let show pos = Format.asprintf "%a" pp pos

let compare : t -> t -> int = compare

let dummy = { pos_lnum = 0; pos_bol = 0; pos_offset = -1 }

let is_dummy = equal dummy

let beg_of_file = { pos_lnum = 1; pos_bol = 0; pos_offset = 0 }

(* constructors *)

let of_line_column_offset ~line ~column ~offset =
  { pos_lnum = line; pos_bol = offset - column; pos_offset = offset }

let of_lexing_pos lp =
  {
    pos_lnum = lp.Lexing.pos_lnum;
    pos_bol = lp.Lexing.pos_bol;
    pos_offset = lp.Lexing.pos_cnum;
  }

let of_lnum_bol_offset ~pos_lnum ~pos_bol ~pos_offset =
  { pos_lnum; pos_bol; pos_offset }

(* accessors *)

let offset t = t.pos_offset

let line t = t.pos_lnum

let column t = t.pos_offset - t.pos_bol

let beg_of_line t = t.pos_bol

let set_column c p =
  { pos_lnum = p.pos_lnum; pos_bol = p.pos_bol; pos_offset = p.pos_bol + c }

let line_beg t = (t.pos_lnum, t.pos_bol)

let line_column t = (t.pos_lnum, t.pos_offset - t.pos_bol)

let line_column_beg t = (t.pos_lnum, t.pos_offset - t.pos_bol, t.pos_bol)

let line_column_offset t = (t.pos_lnum, t.pos_offset - t.pos_bol, t.pos_offset)

let line_beg_offset t = (t.pos_lnum, t.pos_bol, t.pos_offset)

let to_lexing_pos pos_fname t =
  {
    Lexing.pos_fname;
    Lexing.pos_lnum = t.pos_lnum;
    Lexing.pos_bol = t.pos_bol;
    Lexing.pos_cnum = t.pos_offset;
  }
