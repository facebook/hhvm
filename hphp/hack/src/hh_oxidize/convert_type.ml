(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Asttypes
open Longident
open Parsetree
open Printf
open Reordered_argument_collections
open Utils
open State

(* A list of (<module>, <ty1>, <ty2>) tuples where we need to add indirection.
   In the definition of <module>::<ty1>, instances of <ty2> need to be boxed
   (for instances of mutual recursion where we would otherwise define types of
   infinite size). *)
let add_box_between =
  [
    ("typing_defs_core", "ConstraintType", "ConstraintType_");
    ("typing_defs_core", "Ty", "Ty_");
    ("aast_defs", "Hint", "Hint_");
  ]

let should_add_box ty =
  List.mem add_box_between (curr_module_name (), self (), ty) ~equal:( = )

let add_rc_between = [("file_info", "Pos", "relative_path::RelativePath")]

let should_add_rc ty =
  List.mem add_rc_between (curr_module_name (), self (), ty) ~equal:( = )

(* These types inherently add an indirection, so we don't need to box instances
   of recursion in their type arguments. *)
let indirection_types = SSet.of_list ["Vec"]

let rec core_type ?(seen_indirection = false) ct =
  match ct.ptyp_desc with
  | Ptyp_var name -> convert_type_name name
  | Ptyp_alias (_, name) -> convert_type_name name
  | Ptyp_tuple tys -> tuple tys
  | Ptyp_arrow _ -> raise (Skip_type_decl "it contains an arrow type")
  | Ptyp_constr (id, args) ->
    let id =
      match id.txt with
      | Lident "unit" -> "()"
      | Lident "int" -> "isize"
      | Lident "bool" -> "bool"
      | Lident "float" -> "f64"
      | Lident "list" -> "Vec"
      | Lident "ref" -> "std::cell::RefCell"
      | Ldot (Lident "Path", "t") -> "std::path::PathBuf"
      | id -> Convert_longident.longident_to_string id
    in
    let id =
      if id = "T" then
        self ()
      else
        id
    in
    let seen_indirection = seen_indirection || SSet.mem indirection_types id in
    let args =
      (* HACK: eliminate phase type arguments *)
      match args with
      | [{ ptyp_desc = Ptyp_var "phase"; _ }]
      | [{ ptyp_desc = Ptyp_var "ty"; _ }]
      | [{ ptyp_desc = Ptyp_constr ({ txt = Lident "decl_phase"; _ }, _); _ }]
      | [{ ptyp_desc = Ptyp_constr ({ txt = Lident "locl_phase"; _ }, _); _ }]
        ->
        []
      | _ when id = "FunType" -> []
      | _ -> args
    in
    let args = type_args ~seen_indirection args in
    if should_add_rc id then
      sprintf "ocamlrep::rc::RcOc<%s%s>" id args
    (* Direct or indirect recursion *)
    else if (not seen_indirection) && (self () = id || should_add_box id) then
      sprintf "Box<%s%s>" id args
    else
      id ^ args
  | Ptyp_any -> raise (Skip_type_decl "cannot convert type Ptyp_any")
  | Ptyp_object _ -> raise (Skip_type_decl "cannot convert type Ptyp_object")
  | Ptyp_class _ -> raise (Skip_type_decl "cannot convert type Ptyp_class")
  | Ptyp_variant _ -> raise (Skip_type_decl "cannot convert type Ptyp_variant")
  | Ptyp_poly _ -> raise (Skip_type_decl "cannot convert type Ptyp_poly")
  | Ptyp_package _ -> raise (Skip_type_decl "cannot convert type Ptyp_package")
  | Ptyp_extension _ ->
    raise (Skip_type_decl "cannot convert type Ptyp_extension")

and type_args ?(seen_indirection = false) args =
  if List.is_empty args then
    ""
  else
    args
    |> map_and_concat ~f:(core_type ~seen_indirection) ~sep:", "
    |> sprintf "<%s>"

and tuple ?(seen_indirection = false) ?(pub = false) types =
  if List.is_empty types then
    ""
  else
    let map_type =
      if pub then
        fun x ->
      "pub " ^ core_type ~seen_indirection x
      else
        core_type ~seen_indirection
    in
    types |> map_and_concat ~f:map_type ~sep:", " |> sprintf "(%s)"
