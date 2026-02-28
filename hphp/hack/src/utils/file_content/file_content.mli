(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Position : sig
  type t [@@deriving eq, ord, show]

  val beginning_of_file : t

  val from_one_based : int -> int -> t

  val from_zero_based : int -> int -> t

  val line_column_one_based : t -> int * int

  val line_column_zero_based : t -> int * int

  val beginning_of_line : t -> t

  val beginning_of_next_line : t -> t

  val is_beginning_of_line : t -> bool

  val move_back : t -> int -> t

  val to_string_one_based : t -> string
end

type range = {
  st: Position.t;
  ed: Position.t;
}

type text_edit = {
  range: range option;
  text: string;
}

val edit_file :
  string -> text_edit list -> (string, string * Exception.t) result

val edit_file_unsafe : string -> text_edit list -> string

(* NOTE: If you need two offsets, use `get_offsets` below instead. *)
val get_offset : string -> Position.t -> int

(* May raise Invalid_argument "out of bounds" if out of bounds *)
val get_offsets : string -> Position.t * Position.t -> int * int

val offset_to_position : string -> int -> Position.t

val get_char : string -> int -> char
