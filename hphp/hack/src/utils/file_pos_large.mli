(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This `.mli` file was generated automatically. It may include extra
   definitions that should not actually be exposed to the caller. If you notice
   that this interface file is a poor interface, please take a few minutes to
   clean it up manually, and then delete this comment once the interface is in
   shape. *)

type t = {
  pos_lnum: int;  (** line number. Starts at 1. *)
  pos_bol: int;
      (** character number of the beginning of line of this position.
          The column number is therefore offset - bol.
          Starts at 0. *)
  pos_offset: int;
      (** character offset from the beginning of the file. Starts at 0. *)
}
[@@deriving eq, hash, ord, show]

val dummy : t

val is_dummy : t -> bool

val beg_of_file : t

val of_line_column_offset : line:int -> column:int -> offset:int -> t

val of_lexing_pos : Lexing.position -> t

val of_lnum_bol_offset : pos_lnum:int -> pos_bol:int -> pos_offset:int -> t

val offset : t -> int

val line : t -> int

val column : t -> int

val beg_of_line : t -> int

val set_column : int -> t -> t

val line_beg : t -> int * int

val line_column : t -> int * int

val line_column_beg : t -> int * int * int

val line_column_offset : t -> int * int * int

val line_beg_offset : t -> int * int * int

val to_lexing_pos : string -> t -> Lexing.position
