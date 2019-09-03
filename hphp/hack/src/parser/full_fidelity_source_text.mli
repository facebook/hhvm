(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  file_path: Relative_path.t;
  length: int;
  text: string;
  offset_map: Line_break_map.t;
}
[@@deriving show]

type pos = t * int

val make : Relative_path.t -> string -> t
(** create a new source_text.t with a path and contents *)

val empty : t
(** empty source_text.t located nowhere *)

val from_file : Relative_path.t -> t
(** read a relative path into a source_text.t with the contents at that path *)

val file_path : t -> Relative_path.t
(** get the relative path *)

val length : t -> int
(** get the length of the contents *)

val get : t -> int -> char
(** get the ith character *)

val text : t -> string
(** get the contents as a string *)

val line_text : t -> int -> string
(** get just one line as a string *)

val sub : t -> int -> int -> string
(** get a substring start at the ith char and continuing for length *)

val offset_to_position : t -> int -> int * int
(** convert an absolute offset into a (line number, column) pair *)

val position_to_offset : t -> int * int -> int
(** convert a (line number, column) pair into an absolute offset *)

val relative_pos : Relative_path.t -> t -> int -> int -> Pos.t
(** construct a relative position associated with the source_text.t virtual file *)
