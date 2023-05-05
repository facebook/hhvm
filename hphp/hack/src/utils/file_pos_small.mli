(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = int [@@deriving eq, hash]

val dummy : t

val is_dummy : t -> bool

val beg_of_line : t -> int

val line : t -> int

val column : t -> int

val pp : Format.formatter -> t -> unit

val compare : t -> t -> int

val beg_of_file : t

val of_lexing_pos : Lexing.position -> t option

val of_lnum_bol_offset :
  pos_lnum:int -> pos_bol:int -> pos_offset:int -> t option

val offset : t -> int

val line_beg : t -> int * int

val line_column : t -> int * int

val line_column_beg : t -> int * int * int

val line_column_offset : t -> int * int * int

val line_beg_offset : t -> int * int * int

val set_column_unchecked : int -> t -> t

val set_column : int -> t -> t option

val to_lexing_pos : string -> t -> Lexing.position

val as_large_pos : t -> File_pos_large.t

val of_large_pos : File_pos_large.t -> t option
