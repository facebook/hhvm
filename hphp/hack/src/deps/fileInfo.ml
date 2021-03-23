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
  | Mpartial  (** Don't fail if you see a function/class you don't know *)
[@@deriving eq, show, enum]

let parse_mode = function
  | "strict"
  | "" ->
    Some Mstrict
  | "partial" -> Some Mpartial
  | _ -> None

let is_strict = function
  | Mstrict -> true
  | Mhhi
  | Mpartial ->
    false

let string_of_mode = function
  | Mhhi -> "hhi"
  | Mstrict -> "strict"
  | Mpartial -> "partial"

let pp_mode fmt mode =
  Format.pp_print_string fmt
  @@
  match mode with
  | Mhhi -> "Mhhi"
  | Mstrict -> "Mstrict"
  | Mpartial -> "Mpartial"

(*****************************************************************************)
(* Positions of names in a file *)
(*****************************************************************************)

type name_type =
  | Fun [@value 0]
  | Class [@value 1]
  | RecordDef [@value 2]
  | Typedef [@value 3]
  | Const [@value 4]
[@@deriving eq, show, enum, ord]

(** We define two types of positions establishing the location of a given name:
 * a Full position contains the exact position of a name in a file, and a
 * File position contains just the file and the type of toplevel entity,
 * allowing us to lazily retrieve the name's exact location if necessary.
 *)
type pos =
  | Full of Pos.t
  | File of name_type * Relative_path.t
[@@deriving eq, show]

type id = pos * string [@@deriving eq, show]

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)

type hash_type = Int64.t option [@@deriving eq]

let pp_hash_type fmt hash =
  match hash with
  | None -> Format.fprintf fmt "None"
  | Some hash -> Format.fprintf fmt "Some (%s)" (Int64.to_string hash)

(** The record produced by the parsing phase. *)
type t = {
  hash: hash_type;
  file_mode: mode option;
  funs: id list;
  classes: id list;
  record_defs: id list;
  typedefs: id list;
  consts: id list;
  comments: (Pos.t * comment) list option;
      (** None if loaded from saved state *)
}
[@@deriving show]

let empty_t =
  {
    hash = None;
    file_mode = None;
    funs = [];
    classes = [];
    record_defs = [];
    typedefs = [];
    consts = [];
    comments = Some [];
  }

let pos_full (p, name) = (Full p, name)

let get_pos_filename = function
  | Full p -> Pos.filename p
  | File (_, fn) -> fn

(*****************************************************************************)
(* The simplified record used after parsing. *)
(*****************************************************************************)

(** The simplified record used after parsing. *)
type names = {
  n_funs: SSet.t;
  n_classes: SSet.t;
  n_record_defs: SSet.t;
  n_types: SSet.t;
  n_consts: SSet.t;
}

(** The simplified record stored in saved-state.*)
type saved_names = {
  sn_funs: SSet.t;
  sn_classes: SSet.t;
  sn_record_defs: SSet.t;
  sn_types: SSet.t;
  sn_consts: SSet.t;
}

(** Data structure stored in the saved state *)
type saved = {
  s_names: saved_names;
  s_hash: Int64.t option;
  s_mode: mode option;
}

let empty_names =
  {
    n_funs = SSet.empty;
    n_classes = SSet.empty;
    n_record_defs = SSet.empty;
    n_types = SSet.empty;
    n_consts = SSet.empty;
  }

(*****************************************************************************)
(* Functions simplifying the file information. *)
(*****************************************************************************)

let name_set_of_idl idl =
  List.fold_left idl ~f:(fun acc (_, x) -> SSet.add x acc) ~init:SSet.empty

let simplify info =
  let {
    funs;
    classes;
    record_defs;
    typedefs;
    consts;
    file_mode = _;
    comments = _;
    hash = _;
  } =
    info
  in
  let n_funs = name_set_of_idl funs in
  let n_classes = name_set_of_idl classes in
  let n_record_defs = name_set_of_idl record_defs in
  let n_types = name_set_of_idl typedefs in
  let n_consts = name_set_of_idl consts in
  { n_funs; n_classes; n_record_defs; n_types; n_consts }

let to_saved info =
  let {
    funs;
    classes;
    record_defs;
    typedefs;
    consts;
    file_mode = s_mode;
    hash = s_hash;
    comments = _;
  } =
    info
  in
  let sn_funs = name_set_of_idl funs in
  let sn_classes = name_set_of_idl classes in
  let sn_record_defs = name_set_of_idl record_defs in
  let sn_types = name_set_of_idl typedefs in
  let sn_consts = name_set_of_idl consts in
  let s_names = { sn_funs; sn_classes; sn_record_defs; sn_types; sn_consts } in
  { s_names; s_mode; s_hash }

let from_saved fn saved =
  let { s_names; s_mode; s_hash } = saved in
  let { sn_funs; sn_classes; sn_record_defs; sn_types; sn_consts } = s_names in
  let funs = List.map (SSet.elements sn_funs) (fun x -> (File (Fun, fn), x)) in
  let classes =
    List.map (SSet.elements sn_classes) (fun x -> (File (Class, fn), x))
  in
  let record_defs =
    List.map (SSet.elements sn_record_defs) (fun x -> (File (RecordDef, fn), x))
  in
  let typedefs =
    List.map (SSet.elements sn_types) (fun x -> (File (Typedef, fn), x))
  in
  let consts =
    List.map (SSet.elements sn_consts) (fun x -> (File (Const, fn), x))
  in
  {
    file_mode = s_mode;
    hash = s_hash;
    funs;
    classes;
    record_defs;
    typedefs;
    consts;
    comments = None;
  }

let saved_to_names saved =
  {
    n_funs = saved.s_names.sn_funs;
    n_classes = saved.s_names.sn_classes;
    n_record_defs = saved.s_names.sn_record_defs;
    n_types = saved.s_names.sn_types;
    n_consts = saved.s_names.sn_consts;
  }

let merge_names t_names1 t_names2 =
  let { n_funs; n_classes; n_record_defs; n_types; n_consts } = t_names1 in
  {
    n_funs = SSet.union n_funs t_names2.n_funs;
    n_classes = SSet.union n_classes t_names2.n_classes;
    n_record_defs = SSet.union n_record_defs t_names2.n_record_defs;
    n_types = SSet.union n_types t_names2.n_types;
    n_consts = SSet.union n_consts t_names2.n_consts;
  }

let print_names name =
  Printf.printf "Funs:\n";
  SSet.iter (Printf.printf "\t%s\n") name.n_funs;
  Printf.printf "Classes:\n";
  SSet.iter (Printf.printf "\t%s\n") name.n_classes;
  Printf.printf "Types:\n";
  SSet.iter (Printf.printf "\t%s\n") name.n_types;
  Printf.printf "Consts:\n";
  SSet.iter (Printf.printf "\t%s\n") name.n_consts;
  Printf.printf "\n";
  Out_channel.flush stdout;
  ()

let to_string fast =
  [
    ("funs", fast.funs);
    ("classes", fast.classes);
    ("typedefs", fast.typedefs);
    ("consts", fast.consts);
  ]
  |> List.filter ~f:(fun (_, l) -> not @@ List.is_empty l)
  |> List.map ~f:(fun (kind, l) ->
         Printf.sprintf
           "%s: %s"
           kind
           (List.map l ~f:snd |> String.concat ~sep:","))
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
}

let diff f1 f2 =
  let matches_hash =
    match (f1.hash, f2.hash) with
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
        }
