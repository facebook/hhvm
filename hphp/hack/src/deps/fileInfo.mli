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
  | Mphp (* Do the best you can to support legacy PHP *)
  | Mdecl (* just declare signatures, don't check anything *)
  | Mstrict (* check everything! *)
  | Mpartial (* Don't fail if you see a function/class you don't know *)
[@@deriving eq, show, enum]

val is_strict : mode -> bool

val parse_mode : string -> mode option

val string_of_mode : mode -> string

val is_hh_file : mode -> bool

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)

type name_type =
  | Fun [@value 0]
  | Class [@value 1]
  | RecordDef [@value 2]
  | Typedef [@value 3]
  | Const [@value 4]
[@@deriving eq, show, enum]

type pos =
  | Full of Pos.t
  | File of name_type * Relative_path.t
[@@deriving eq, show]

type id = pos * string [@@deriving eq, show]

val pos_full : Pos.t * string -> id

val get_pos_filename : pos -> Relative_path.t

type t = {
  hash: OpaqueDigest.t option;
  file_mode: mode option;
  funs: id list;
  classes: id list;
  record_defs: id list;
  typedefs: id list;
  consts: id list;
  comments: (Pos.t * comment) list option;
}
[@@deriving eq, show]

val empty_t : t

(*****************************************************************************)
(* The simplified record used after parsing. *)
(*****************************************************************************)
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
