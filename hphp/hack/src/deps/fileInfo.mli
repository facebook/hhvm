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

open Prim_defs

(*****************************************************************************)
(* Parsing modes *)
(*****************************************************************************)

(* TODO(t16719394): kill off file_type *)
type file_type =
  | PhpFile
  | HhFile

type mode =
  | Mphp     (* Do the best you can to support legacy PHP *)
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
  hash : Digest.t option;
  file_mode : mode option;
  funs : id list;
  classes : id list;
  typedefs : id list;
  consts : id list;
  comments : (Pos.t * comment) list option;
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


(*****************************************************************************)
(* The record used in our saved state. *)
(*****************************************************************************)
type saved
type saved_state_info = saved Relative_path.Map.t


val empty_names: names

(*****************************************************************************)
(* Functions simplifying the file information. *)
(*****************************************************************************)
val saved_to_info : saved_state_info -> t Relative_path.Map.t
val saved_to_fast: saved_state_info -> fast
val saved_to_hack_files: saved_state_info -> fast
val info_to_saved : t Relative_path.Map.t -> saved_state_info
val simplify: t -> names
val merge_names: names -> names -> names
val simplify_fast: t Relative_path.Map.t -> fast
val print_names : names -> unit
