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

open Hh_core
open Prim_defs

(*****************************************************************************)
(* Parsing modes *)
(*****************************************************************************)

type file_type =
  | PhpFile
  | HhFile

type mode =
  | Mphp     (* Do the best you can to support legacy PHP *)
  | Mdecl    (* just declare signatures, don't check anything *)
  | Mstrict  (* check everthing! *)
  | Mpartial (* Don't fail if you see a function/class you don't know *)


(*****************************************************************************)
(* We define two types of positions establishing the location of a given name:
 * a Full position contains the exact position of a name in a file, and a
 * File position contains just the file and the type of toplevel entity,
 * allowing us to lazily retrieve the name's exact location if necessary.
 *)
(*****************************************************************************)
type name_type = Fun | Class | Typedef | Const
type pos = Full of Pos.t | File of name_type * Relative_path.t
type id = pos  * string
(* The hash value of a decl AST.
  We use this to see if two versions of a file are "similar", i.e. their
  declarations only differ by position information.  *)

(*****************************************************************************)
(* The record produced by the parsing phase. *)
(*****************************************************************************)
type t = {
  hash : Digest.t option;
  file_mode : mode option;
  funs : id list;
  classes : id list;
  typedefs : id list;
  consts : id list;
  comments : (Pos.t * comment) list option;
    (* None if loaded from saved state *)
}

let empty_t = {
  hash = None;
  file_mode = None;
  funs = [];
  classes = [];
  typedefs = [];
  consts = [];
  comments = Some [];
}

let pos_full (p, name) =
  Full p, name

let get_pos_filename = function
| Full p -> Pos.filename p
| File (_, fn) -> fn

(*****************************************************************************)
(* The simplified record used after parsing. *)
(*****************************************************************************)

type names = {
  n_funs    : SSet.t;
  n_classes : SSet.t;
  n_types   : SSet.t;
  n_consts  : SSet.t;
}

(* Data structure stored in the saved state *)
type saved = {
  s_names : names;
  s_hash : Digest.t option;
  s_mode: mode option;
}


type fast = names Relative_path.Map.t

(* Object we get from saved state *)
type saved_state_info = saved Relative_path.Map.t

let empty_names = {
  n_funs    = SSet.empty;
  n_classes = SSet.empty;
  n_types   = SSet.empty;
  n_consts  = SSet.empty;
}

(*****************************************************************************)
(* Functions simplifying the file information. *)
(*****************************************************************************)

let name_set_of_idl idl =
  List.fold_left idl ~f:(fun acc (_, x) -> SSet.add x acc) ~init:SSet.empty

let saved_to_fast fast =
  Relative_path.Map.map fast
  begin fun saved -> saved.s_names end

(* Filter out all PHP files from saved fileInfo object. *)
let saved_to_hack_files fast =
  saved_to_fast fast


let saved_to_info fast =
  Relative_path.Map.fold fast ~init:Relative_path.Map.empty
  ~f:(fun fn saved acc ->
    let {s_names; s_mode; s_hash} = saved in
    let {n_funs; n_classes; n_types; n_consts;} = s_names in
    let funs = List.map (SSet.elements n_funs)
      (fun x -> File (Fun, fn), x) in
    let classes = List.map (SSet.elements n_classes)
      (fun x -> File (Class, fn), x) in
    let typedefs = List.map (SSet.elements n_types)
      (fun x -> File (Typedef, fn), x) in
    let consts = List.map (SSet.elements n_consts)
      (fun x -> File (Const, fn), x) in
    let fileinfo = {
      file_mode= s_mode;
      hash = s_hash;
      funs;
      classes;
      typedefs;
      consts;
      comments = None;
    } in
    Relative_path.Map.add acc fn fileinfo
  )

let info_to_saved fileinfo =
  Relative_path.Map.map fileinfo
    (fun info ->
      let {funs; classes; typedefs; consts; file_mode= s_mode;
          hash= s_hash; comments = _; } = info in
      let n_funs    = name_set_of_idl funs in
      let n_classes = name_set_of_idl classes in
      let n_types   = name_set_of_idl typedefs in
      let n_consts  = name_set_of_idl consts in

      let s_names = { n_funs; n_classes; n_types; n_consts; } in
      { s_names; s_mode; s_hash; })

let simplify info =
  let {funs; classes; typedefs; consts; file_mode = _; comments = _;
      hash = _;} = info in
  let n_funs    = name_set_of_idl funs in
  let n_classes = name_set_of_idl classes in
  let n_types   = name_set_of_idl typedefs in
  let n_consts  = name_set_of_idl consts in
  {n_funs; n_classes; n_types; n_consts}

let merge_names t_names1 t_names2 =
  let {n_funs; n_classes; n_types; n_consts} = t_names1 in
  {
   n_funs    = SSet.union n_funs t_names2.n_funs;
   n_classes = SSet.union n_classes t_names2.n_classes;
   n_types   = SSet.union n_types t_names2.n_types;
   n_consts  = SSet.union n_consts t_names2.n_consts;
  }

let simplify_fast fast =
  Relative_path.Map.map fast simplify

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
  flush stdout;
  ()
