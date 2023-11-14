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
[@@deriving eq, hash, show, enum, ord, sexp_of]

val is_strict : mode -> bool

val is_hhi : mode -> bool

val string_of_mode : mode -> string

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)

(** This type replicates what's in Naming_types.name_kind, but with less structure.
It'd be nice to unify them. *)
type name_type =
  | Fun [@value 3]
  | Class [@value 0]
  | Typedef [@value 1]
  | Const [@value 4]
  | Module [@value 5]
[@@deriving eq, show, enum, ord]

(** And here's a version with more detail and still less structure! This
one is good for members as well as top-level symbols. It's used to get
a bit more out of direct-decl-parse, used to populate the search indexer
(e.g. the icon that apepars in autocomplete suggestions). *)
type si_kind =
  | SI_Class
  | SI_Interface
  | SI_Enum
  | SI_Trait
  | SI_Unknown
  | SI_Mixed
  | SI_Function
  | SI_Typedef
  | SI_GlobalConstant
  | SI_XHP
  | SI_Namespace
  | SI_ClassMethod
  | SI_Literal
  | SI_ClassConstant
  | SI_Property
  | SI_LocalVariable
  | SI_Keyword
  | SI_Constructor
[@@deriving eq, show { with_path = false }]

(** Yet more details. The "is_abstract" and "is_final" are used e.g. for autocomplete
items and to determine whether a type can be suggested e.g. for "$x = new |". *)
type si_addendum = {
  sia_name: string;
      (** This is expected not to contain the leading namespace backslash! See [Utils.strip_ns]. *)
  sia_kind: si_kind;
  sia_is_abstract: bool;
  sia_is_final: bool;
}
[@@deriving show]

type pos =
  | Full of Pos.t
  | File of name_type * Relative_path.t
[@@deriving eq, show]

type id = pos * string * Int64.t option [@@deriving eq, show]

val id_name : id -> string

val pos_full : Pos.t * string * Int64.t option -> id

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
  typedefs: id list;
  consts: id list;
  modules: id list;
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
  n_types: SSet.t;
  n_consts: SSet.t;
  n_modules: SSet.t;
}
[@@deriving show]

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
  removed_modules: SSet.t;
  added_modules: SSet.t;
}

val diff : t -> t -> diff option
