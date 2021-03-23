(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* This module defines the data structured used to describe the content of
 * a file.
 * The parser constructs FileInfo.t structs, that contain names and positions
 * plus some extra info required for the build.
 * After the names have been checked (Naming.make_env), we "simplify" the
 * struct and only keep the names defined in the files we know about.
 *)
(*****************************************************************************)

open Prim_defs

(*****************************************************************************)
(* Parsing modes *)
(*****************************************************************************)

type mode =
  | Mhhi (* just declare signatures, don't check anything *)
  | Mstrict (* check everything! *)
  | Mpartial (* Don't fail if you see a function/class you don't know *)
[@@deriving eq, show, enum]

val is_strict : mode -> bool

val parse_mode : string -> mode option

val string_of_mode : mode -> string

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)

type name_type =
  | Fun [@value 0]
  | Class [@value 1]
  | RecordDef [@value 2]
  | Typedef [@value 3]
  | Const [@value 4]
[@@deriving eq, show, enum, ord]

type pos =
  | Full of Pos.t
  | File of name_type * Relative_path.t
[@@deriving eq, show]

type id = pos * string [@@deriving eq, show]

val pos_full : Pos.t * string -> id

val get_pos_filename : pos -> Relative_path.t

(** The hash value of a decl AST.
  We use this to see if two versions of a file are "similar", i.e. their
  declarations only differ by position information.  *)
type hash_type = Int64.t option [@@deriving eq]

(** [FileInfo.t] is (1) what we get out of the parser, with Full positions;
(2) the API for putting stuff into and taking stuff out of saved-state naming table (with File positions)
*)
type t = {
  hash: hash_type;
  file_mode: mode option;
  funs: id list;
  classes: id list;
  record_defs: id list;
  typedefs: id list;
  consts: id list;
  comments: (Pos.t * comment) list option;
}
[@@deriving show]

val empty_t : t

(*****************************************************************************)
(* The simplified record used after parsing. *)
(*****************************************************************************)

(** [FileInfo.names] is a cut-down version of [FileInfo.t], one that we use internally
for decl-diffing and other fanout calculations. *)
type names = {
  n_funs: SSet.t;
  n_classes: SSet.t;
  n_record_defs: SSet.t;
  n_types: SSet.t;
  n_consts: SSet.t;
}

(*****************************************************************************)
(* The record used in our saved state. *)
(*****************************************************************************)

(** Although [FileInfo.t] is the public API for storing/retrieving entries in the naming-table,
we actually store the naming-table on disk as [FileInfo.saved] - it's basically the same but
has a slightly more compact representation in order to save space. *)
type saved

val empty_names : names

(*****************************************************************************)
(* Functions simplifying the file information. *)
(*****************************************************************************)
val simplify : t -> names

val merge_names : names -> names -> names

val print_names : names -> unit

val to_saved : t -> saved

val from_saved : Relative_path.t -> saved -> t

val saved_to_names : saved -> names

val to_string : t -> string

type diff = {
  removed_funs: SSet.t;
  added_funs: SSet.t;
  removed_classes: SSet.t;
  added_classes: SSet.t;
  removed_types: SSet.t;
  added_types: SSet.t;
  removed_consts: SSet.t;
  added_consts: SSet.t;
}

val diff : t -> t -> diff option
