(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** There are two kinds of positions: AST positions provided by the parser,
    which are fully qualified with filename, line and column ranges, and
    decl positions, which may be compressed and need resolving before being
    used or printed.

    AST positions are represented by Pos.t. Decl positions don't have their own
    type yet but may have in the future.
    This type is for either of these positions.
    It's used in the decl heap and in places that can take any kind of positions,
    e.g. error secondary positions. *)
type t [@@deriving eq, ord, show]

module Map : WrappedMap.S with type key = t

(** The decl and file of a position. *)
type ctx = {
  decl: Decl_reference.t option;
  file: Relative_path.t;
}
[@@deriving show]

val none : t

(** Fill in the gap "between" first position and second position.
    Not valid if from different files or second position precedes first *)
val btw : t -> t -> t

(** Essentially an upcast. *)
val of_raw_pos : Pos.t -> t

val set_from_reason : t -> t

(** Compress a position to be stored in the decl heap. *)
val make_decl_pos : Pos.t -> Decl_reference.t -> t

(** Compress a position to be stored in the decl heap.
    If no decl reference is given, the position is not compressed but
    simply converted. *)
val make_decl_pos_of_option : Pos.t -> Decl_reference.t option -> t

val is_hhi : t -> bool

val filename : t -> Relative_path.t

(** This may become unsafe in the future as we change the implementation
    of positions in the decl heap. Avoid using in new code.
    Use a position from an AST instead of from a decl or type,
    or resolve decl position to a raw position using a provider context.
    TODO: get rid of unsafe_to_raw_pos before changing implementation of t. T87777740 *)
val unsafe_to_raw_pos : t -> Pos.t

(** For spans over just one line, return the line number, start column and end column.
    This returns a closed interval.
    Undefined for multi-line spans. *)
val line_start_end_columns : t -> int * int * int

val json : t -> Hh_json.json

val show_as_absolute_file_line_characters : t -> string

(** Replace the decl reference part of the position with a filename. *)
val fill_in_filename : Relative_path.t -> t -> Pos.t

(** Check that the position is in the current decl and if it is, resolve
    it with the current file. *)
val fill_in_filename_if_in_current_decl :
  current_decl_and_file:ctx -> t -> Pos.t option

(** Returns either a raw position equivalent to this position or the decl
    that this position belongs to. *)
val get_raw_pos_or_decl_reference :
  t -> [> `Raw of Pos.t | `Decl_ref of Decl_reference.t ]

val to_span : t -> unit Pos.pos
