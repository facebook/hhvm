(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Hhbc_string_utils

module A = Ast
module TV = Typed_value

(* Taken from: hphp/runtime/base/type-structure.h *)
let get_kind_num ~tparams p =
  let p = if List.mem tparams p then "typevar" else String.lowercase_ascii p in
  Int64.of_int @@
  match p with
  | "void" -> 0
  | "int" | "integer" -> 1
  | "bool" | "boolean" -> 2
  | "float" | "double" | "real" -> 3
  | "string" -> 4
  | "resource" -> 5
  | "num" -> 6
  | "noreturn" -> 8
  | "arraykey" -> 7
  | "mixed" -> 9
  | "tuple" -> 10
  | "fun" -> 11
  | "array" -> 12
  | "typevar" -> 13 (* corresponds to user OF_GENERIC *)
  | "shape" -> 14
  | "class" -> 15
  | "interface" -> 16
  | "trait" -> 17
  | "enum" -> 18
  | "dict" -> 19
  | "vec" -> 20
  | "keyset" -> 21
  | "typeaccess" -> 102
  | "xhp" -> 103
  | "unresolved"
  | _ -> 101

and in_HH_namespace = function
  | "Vector" | "ImmVector"
  | "Set" | "ImmSet"
  | "Map" | "ImmMap"
  | "Pair" | "this" -> true
  | _ -> false

and is_prim s = match String.lowercase_ascii s with
  | "void" | "int" | "integer"
  | "bool" | "boolean"
  | "float" | "double" | "real"
  | "string" | "resource"
  | "num" | "noreturn"
  | "arraykey" | "mixed" -> true
  | _ -> false

and is_resolved_classname s = match String.lowercase_ascii s with
  | "array" | "vec"
  | "dict" | "keyset" -> true
  | _ -> false

let shape_field_name = function
  | A.SFlit ((_, s)) -> s, false
  | A.SFclass_const ((_, id), (_, s)) -> id ^ "::" ^ s, true

let rec shape_field_to_pair ~tparams sf =
  let name, is_class_const = shape_field_name sf.A.sf_name in
  let is_optional = sf.A.sf_optional in
  let hint = sf.A.sf_hint in
  let class_const =
    if is_class_const then [(TV.String "is_cls_cns", TV.Bool true)] else []
  in
  let optional =
    if is_optional
    then [(TV.String "optional_shape_field", TV.Bool true)] else []
  in
  let inner_value =
    [(TV.String "value", hint_to_type_constant ~tparams hint)]
  in
  let inner_value = class_const @ optional @ inner_value in
  let value = TV.Array inner_value in
  (TV.String name, value)

and shape_info_to_typed_value ~tparams si =
  TV.Array (List.map ~f:(shape_field_to_pair ~tparams) si.A.si_shape_field_list)

and shape_allows_unknown_fields { A.si_allows_unknown_fields; _ } =
  if si_allows_unknown_fields then
    [TV.String "allows_unknown_fields", TV.Bool true]
  else []

and type_constant_access_list sl =
  let l =
    List.mapi ~f:(fun i (_, s) -> (TV.Int (Int64.of_int i), TV.String s)) sl
  in
  TV.Array l

and resolve_classname ~tparams s =
  if is_prim s || is_resolved_classname s then []
  else
    let s = Types.fix_casing s in
    let name = if in_HH_namespace s then prefix_namespace "HH" s else s in
    let id = if List.mem tparams name then "name" else "classname" in
    [TV.String id, TV.String name]

and get_generic_types ~tparams = function
  | [] -> []
  | l -> [TV.String "generic_types", hints_to_type_constant ~tparams l]

and get_kind ~tparams s = [TV.String "kind", TV.Int (get_kind_num ~tparams s)]

and root_to_string s =
  if s = "this" then prefix_namespace "HH" s
  else s

and get_typevars = function
 | [] -> []
 | tparams -> [TV.String "typevars", TV.String (String.concat "," tparams)]

and hint_to_type_constant_list ~tparams h =
  match snd h with
  | A.Happly ((_, s), l) ->
    let kind = get_kind ~tparams s in
    let classname = resolve_classname ~tparams s in
    let generic_types = get_generic_types ~tparams l in
    kind @ classname @ generic_types
  | A.Hshape (si) ->
    shape_allows_unknown_fields si
    @ get_kind ~tparams "shape"
    @ [TV.String "fields", shape_info_to_typed_value ~tparams si]
  | A.Haccess ((_, s0), s1, sl) ->
    get_kind ~tparams "typeaccess" @
     [TV.String "root_name", TV.String (root_to_string s0);
     TV.String "access_list", type_constant_access_list @@ s1::sl]
  | A.Hfun (hl, _b, h) ->
    (* TODO: Bool indicates variadic argument. What to do? *)
    let kind = get_kind ~tparams "fun" in
    let return_type =
      [TV.String "return_type", hint_to_type_constant ~tparams h]
    in
    let param_types =
      [TV.String "param_types", hints_to_type_constant ~tparams hl]
    in
    kind @ return_type @ param_types
  | A.Hoption h ->
    [TV.String "nullable", TV.Bool true]
    @ hint_to_type_constant_list ~tparams h
  | A.Htuple hl ->
    let kind = get_kind ~tparams "tuple" in
    let elem_types =
      [TV.String "elem_types", hints_to_type_constant ~tparams hl]
    in
    kind @ elem_types
  | A.Hsoft h -> hint_to_type_constant_list ~tparams h

and hint_to_type_constant ?(is_typedef = false) ~tparams h =
  let l = hint_to_type_constant_list ~tparams h in
  TV.Array (l @ if is_typedef then get_typevars tparams else [])

and hints_to_type_constant ~tparams l =
  TV.Array (
    List.mapi l
      ~f:(fun i h ->
            (TV.Int (Int64.of_int i),
            hint_to_type_constant ~tparams h)))
