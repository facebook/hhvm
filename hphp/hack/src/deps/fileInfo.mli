(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

(*****************************************************************************)
(* Parsing modes *)
(*****************************************************************************)

type file_type =
  | PhpFile
  | HhFile

type mode =
  | Mdecl    (* just declare signatures, don't check anything *)
  | Mstrict  (* check everthing! *)
  | Mpartial (* Don't fail if you see a function/class you don't know *)

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)

type name_type = Fun | Class | Typedef | Const
type pos = Full of Pos.t | File of name_type * Relative_path.t
type id = pos  * string
val pos_full : (Pos.t * string) -> id
val get_pos_filename : pos -> Relative_path.t
type t = {
  file_mode : mode option;
  funs : id list;
  classes : id list;
  typedefs : id list;
  consts : id list;
  comments : (Pos.t * string) list option;
  consider_names_just_for_autoload: bool;
}

val empty_t: t

(*****************************************************************************)
(* The simplified record used after parsing. *)
(*****************************************************************************)

type names = {
  n_funs    : SSet.t;
  n_classes : SSet.t;
  n_types   : SSet.t;
  n_consts  : SSet.t;
}

type fast = names Relative_path.Map.t
type fast_with_modes = (names * mode option) Relative_path.Map.t


val empty_names: names

(*****************************************************************************)
(* Functions simplifying the file information. *)
(*****************************************************************************)
val modes_to_info : fast_with_modes -> t Relative_path.Map.t
val modes_to_fast: fast_with_modes -> fast
val info_to_modes : t Relative_path.Map.t -> fast_with_modes
val simplify: t -> names
val merge_names: names -> names -> names
val simplify_fast: t Relative_path.Map.t -> fast
val print_names : names -> unit
