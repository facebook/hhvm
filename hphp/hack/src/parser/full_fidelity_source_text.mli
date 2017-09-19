(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t

(** create a new source_text.t with a path and contents *)
val make : Relative_path.t -> string -> t

(** empty source_text.t located nowhere *)
val empty : t

(** read a relative path into a source_text.t with the contents at that path *)
val from_file : Relative_path.t -> t

(** get the contents as a string *)
val text : t -> string

(** get the relative path *)
val file_path : t -> Relative_path.t

(** get the length of the contents *)
val length : t -> int

(** get the contents as a string *)
val get_text : t -> string

(** get the ith character *)
val get : t -> int -> char

(** get a substring start at the ith char and continuing for length *)
val sub : t -> int -> int -> string

(** convert an absolute offset into a (line number, column) pair *)
val offset_to_position : t -> int -> int * int

(** convert a (line number, column) pair into an absolute offset *)
val position_to_offset : t -> int * int -> int

(** construct a relative position associated with the source_text.t virtual file *)
val relative_pos : Relative_path.t -> t -> int -> int -> Pos.t

(** is there a newline at absolute offset i? *)
val is_newline : t -> int -> bool

(** get the absolute offset corresponding to the start of the line containing offset i *)
val start_of_line : t -> int -> int

(** get the absolute offset corresponding to the end of the line containing offset i *)
val end_of_line : t -> int -> int
