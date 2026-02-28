(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A compressed representation of a position span, i.e. a start and an end position. *)

type t [@@deriving eq, hash, show, ord]

val dummy : t

val make : pos_start:File_pos_large.t -> pos_end:File_pos_large.t -> t option

val as_large_span : t -> File_pos_large.t * File_pos_large.t

val start_line_number : t -> int

val start_beginning_of_line : t -> int

val start_column : t -> int

val start_offset : t -> int

val end_line_number : t -> int

val end_beginning_of_line : t -> int

val end_column : t -> int

val end_offset : t -> int
