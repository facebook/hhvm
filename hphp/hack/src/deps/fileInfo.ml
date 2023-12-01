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

open Hh_prelude
open Prim_defs

(*****************************************************************************)
(* Parsing modes *)
(*****************************************************************************)

type mode =
  | Mhhi  (** just declare signatures, don't check anything *)
  | Mstrict  (** check everything! *)
[@@deriving eq, hash, show, enum, ord, sexp_of]

let is_strict = function
  | Mstrict -> true
  | Mhhi -> false

let is_hhi = function
  | Mstrict -> false
  | Mhhi -> true

let string_of_mode = function
  | Mhhi -> "hhi"
  | Mstrict -> "strict"

let pp_mode fmt mode =
  Format.pp_print_string fmt
  @@
  match mode with
  | Mhhi -> "Mhhi"
  | Mstrict -> "Mstrict"

(*****************************************************************************)
(* Positions of names in a file *)
(*****************************************************************************)

type name_type =
  | Fun [@value 3]
  | Class [@value 0]
  | Typedef [@value 1]
  | Const [@value 4]
  | Module [@value 5]
[@@deriving eq, show { with_path = false }, enum, ord]

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

(** We define two types of positions establishing the location of a given name:
 * a Full position contains the exact position of a name in a file, and a
 * File position contains just the file and the type of toplevel entity,
 * allowing us to lazily retrieve the name's exact location if necessary.
 *)
type pos =
  | Full of Pos.t
  | File of name_type * Relative_path.t
[@@deriving eq, show]

(** An id contains a pos, name and a optional decl hash. The decl hash is None
 * only in the case when we didn't compute it for performance reasons
 *)
type id = pos * string * Int64.t option [@@deriving eq, show]

let id_name (_, x, _) = x

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)

type hash_type = Int64.t option [@@deriving eq]

let pp_hash_type fmt hash =
  match hash with
  | None -> Format.fprintf fmt "None"
  | Some hash -> Format.fprintf fmt "Some (%s)" (Int64.to_string hash)

(* NB: Type [t] must be manually kept in sync with Rust type [hackrs_provider_backend::FileInfo] *)

(** The record produced by the parsing phase. *)
type t = {
  position_free_decl_hash: hash_type;
  file_mode: mode option;
  funs: id list;
  classes: id list;
  typedefs: id list;
  consts: id list;
  modules: id list;
  comments: (Pos.t * comment) list option;
      (** None if loaded from saved state *)
}
[@@deriving show]

let empty_t =
  {
    position_free_decl_hash = None;
    file_mode = None;
    funs = [];
    classes = [];
    typedefs = [];
    consts = [];
    modules = [];
    comments = Some [];
  }

let pos_full (p, name, hash) = (Full p, name, hash)

let get_pos_filename = function
  | Full p -> Pos.filename p
  | File (_, fn) -> fn

type pfh_hash = Int64.t

type change = {
  path: Relative_path.t;
  old_file_info: t option;
  new_file_info: t option;
  new_pfh_hash: pfh_hash option;
}

(*****************************************************************************)
(* The simplified record used after parsing. *)
(*****************************************************************************)

(** The simplified record used after parsing. *)
type names = {
  n_funs: SSet.t;
  n_classes: SSet.t;
  n_types: SSet.t;
  n_consts: SSet.t;
  n_modules: SSet.t;
}
[@@deriving show]

(** The simplified record stored in saved-state.*)
type saved_names = {
  sn_funs: SSet.t;
  sn_classes: SSet.t;
  sn_types: SSet.t;
  sn_consts: SSet.t;
  sn_modules: SSet.t;
}

(** Data structure stored in the saved state *)
type saved = {
  s_names: saved_names;
  s_position_free_decl_hash: Int64.t option;
  s_mode: mode option;
}

let empty_names =
  {
    n_funs = SSet.empty;
    n_classes = SSet.empty;
    n_types = SSet.empty;
    n_consts = SSet.empty;
    n_modules = SSet.empty;
  }

(*****************************************************************************)
(* Functions simplifying the file information. *)
(*****************************************************************************)

let name_set_of_idl idl =
  List.fold_left idl ~f:(fun acc (_, x, _) -> SSet.add x acc) ~init:SSet.empty

let simplify info =
  let {
    funs;
    classes;
    typedefs;
    consts;
    modules;
    file_mode = _;
    comments = _;
    position_free_decl_hash = _;
  } =
    info
  in
  let n_funs = name_set_of_idl funs in
  let n_classes = name_set_of_idl classes in
  let n_types = name_set_of_idl typedefs in
  let n_consts = name_set_of_idl consts in
  let n_modules = name_set_of_idl modules in
  { n_funs; n_classes; n_types; n_consts; n_modules }

let to_saved info : saved =
  let {
    funs;
    classes;
    typedefs;
    consts;
    modules;
    file_mode;
    position_free_decl_hash;
    comments = _;
  } =
    info
  in
  let sn_funs = name_set_of_idl funs in
  let sn_classes = name_set_of_idl classes in
  let sn_types = name_set_of_idl typedefs in
  let sn_consts = name_set_of_idl consts in
  let sn_modules = name_set_of_idl modules in
  let s_names = { sn_funs; sn_classes; sn_types; sn_consts; sn_modules } in
  {
    s_names;
    s_mode = file_mode;
    s_position_free_decl_hash = position_free_decl_hash;
  }

let from_saved fn saved =
  let { s_names; s_mode; s_position_free_decl_hash } = saved in
  let { sn_funs; sn_classes; sn_types; sn_consts; sn_modules } = s_names in
  let funs =
    List.map (SSet.elements sn_funs) ~f:(fun x -> (File (Fun, fn), x, None))
  in
  let classes =
    List.map (SSet.elements sn_classes) ~f:(fun x ->
        (File (Class, fn), x, None))
  in
  let typedefs =
    List.map (SSet.elements sn_types) ~f:(fun x ->
        (File (Typedef, fn), x, None))
  in
  let consts =
    List.map (SSet.elements sn_consts) ~f:(fun x -> (File (Const, fn), x, None))
  in
  let modules =
    List.map (SSet.elements sn_modules) ~f:(fun m ->
        (File (Module, fn), m, None))
  in
  {
    file_mode = s_mode;
    position_free_decl_hash = s_position_free_decl_hash;
    funs;
    classes;
    typedefs;
    consts;
    modules;
    comments = None;
  }

let saved_to_names saved =
  {
    n_funs = saved.s_names.sn_funs;
    n_classes = saved.s_names.sn_classes;
    n_types = saved.s_names.sn_types;
    n_consts = saved.s_names.sn_consts;
    n_modules = saved.s_names.sn_modules;
  }

let merge_names t_names1 t_names2 =
  let { n_funs; n_classes; n_types; n_consts; n_modules } = t_names1 in
  {
    n_funs = SSet.union n_funs t_names2.n_funs;
    n_classes = SSet.union n_classes t_names2.n_classes;
    n_types = SSet.union n_types t_names2.n_types;
    n_consts = SSet.union n_consts t_names2.n_consts;
    n_modules = SSet.union n_modules t_names2.n_modules;
  }

let to_string defs_per_file =
  let funs = List.map ~f:(fun (a, b, _) -> (a, b, None)) defs_per_file.funs in
  let classes =
    List.map ~f:(fun (a, b, _) -> (a, b, None)) defs_per_file.classes
  in
  let typedefs =
    List.map ~f:(fun (a, b, _) -> (a, b, None)) defs_per_file.typedefs
  in
  let consts =
    List.map ~f:(fun (a, b, _) -> (a, b, None)) defs_per_file.consts
  in
  let modules =
    List.map ~f:(fun (a, b, _) -> (a, b, None)) defs_per_file.modules
  in
  [
    ("funs", funs);
    ("classes", classes);
    ("typedefs", typedefs);
    ("consts", consts);
    ("modules", modules);
  ]
  |> List.filter ~f:(fun (_, l) -> not @@ List.is_empty l)
  |> List.map ~f:(fun (kind, l) ->
         Printf.sprintf
           "%s: %s %s"
           kind
           (List.map l ~f:(fun (_, x, _) -> x) |> String.concat ~sep:",")
           (List.map l ~f:(fun (_, _, hash) ->
                Int64.to_string (Option.value hash ~default:Int64.zero))
           |> String.concat ~sep:","))
  |> String.concat ~sep:";"

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

let diff f1 f2 =
  let matches_hash =
    match (f1.position_free_decl_hash, f2.position_free_decl_hash) with
    | (Some h1, Some h2) -> Int64.equal h1 h2
    | _ -> false
  in
  if matches_hash then
    None
  else
    let diff_ids ids1 ids2 =
      let removed_ids = SSet.diff ids1 ids2 in
      let added_ids = SSet.diff ids2 ids1 in
      (removed_ids, added_ids)
    in
    let f1 = simplify f1 in
    let f2 = simplify f2 in
    let (removed_funs, added_funs) = diff_ids f1.n_funs f2.n_funs in
    let (removed_classes, added_classes) = diff_ids f1.n_classes f2.n_classes in
    let (removed_types, added_types) = diff_ids f1.n_types f2.n_types in
    let (removed_consts, added_consts) = diff_ids f1.n_consts f2.n_consts in
    let (removed_modules, added_modules) = diff_ids f1.n_modules f2.n_modules in
    let is_empty =
      List.fold
        ~f:(fun acc s -> (not (SSet.is_empty s)) || acc)
        [
          removed_funs;
          added_funs;
          removed_classes;
          added_classes;
          removed_types;
          added_types;
          removed_consts;
          added_consts;
          removed_modules;
          added_modules;
        ]
        ~init:false
    in
    if is_empty then
      None
    else
      Some
        {
          removed_funs;
          added_funs;
          removed_classes;
          added_classes;
          removed_types;
          added_types;
          removed_consts;
          added_consts;
          removed_modules;
          added_modules;
        }
