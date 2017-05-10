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
let get_kind_num p = Int64.of_int @@
  match p with
  | "void" -> 0
  | "int" -> 1
  | "bool" -> 2
  | "float" -> 3
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

and is_prim = function
  | "void" | "int"
  | "bool" | "float"
  | "string" | "resource"
  | "num" | "noreturn"
  | "arraykey" | "mixed" -> true
  | _ -> false

and is_resolved_classname = function
  | "array" | "vec"
  | "dict" | "keyset" -> true
  | _ -> false

let shape_field_name = function
  | A.SFlit ((_, s)) -> s, false
  | A.SFclass_const ((_, id), (_, s)) -> id ^ "::" ^ s, true

let rec shape_field_to_pair sf =
  let name, is_class_const = shape_field_name sf.A.sf_name in
  (* TODO: Optional? *)
  let hint = sf.A.sf_hint in
  let class_const =
    if is_class_const then [(TV.String "is_cls_cns", TV.Bool true)] else []
  in
  let inner_value =
    class_const @ [(TV.String "value", hint_to_type_constant hint)]
  in
  let value = TV.Array inner_value in
  (TV.String name, value)

and shape_info_to_typed_value si =
  TV.Array (List.map ~f:shape_field_to_pair si.A.si_shape_field_list)

and type_constant_access_list sl =
  let l =
    List.mapi ~f:(fun i (_, s) -> (TV.Int (Int64.of_int i), TV.String s)) sl
  in
  TV.Array l

and resolve_classname s =
  if is_prim s || is_resolved_classname s then []
  else
    let classname = if in_HH_namespace s then prefix_namespace "HH" s else s in
     [TV.String "classname", TV.String classname]

and get_generic_types = function
  | [] -> []
  | l -> [TV.String "generic_types", hints_to_type_constant l]

and get_kind s = [TV.String "kind", TV.Int (get_kind_num s)]

and root_to_string s =
  if s = "this" then prefix_namespace "HH" s
  else s

and hint_to_type_constant_list h =
  match snd h with
  | A.Happly ((_, s), l) ->
    let kind = get_kind s in
    let classname = resolve_classname s in
    let generic_types = get_generic_types l in
    kind @ classname @ generic_types
  | A.Hshape (si) ->
    get_kind "shape" @ [TV.String "fields", shape_info_to_typed_value si]
  | A.Haccess ((_, s0), s1, sl) ->
    get_kind "typeaccess" @
     [TV.String "root_name", TV.String (root_to_string s0);
     TV.String "access_list", type_constant_access_list @@ s1::sl]
  | A.Hfun (hl, _b, h) ->
    (* TODO: Bool indicates variadic argument. What to do? *)
    let kind = get_kind "fun" in
    let return_type = [TV.String "return_type", hint_to_type_constant h] in
    let param_types = [TV.String "param_types", hints_to_type_constant hl] in
    kind @ return_type @ param_types
  | A.Hoption h ->
    [TV.String "nullable", TV.Bool true] @ hint_to_type_constant_list h
  | A.Htuple hl ->
    let kind = get_kind "tuple" in
    let elem_types = [TV.String "elem_types", hints_to_type_constant hl] in
    kind @ elem_types
  | A.Hsoft h -> hint_to_type_constant_list h

and hint_to_type_constant h =
  TV.Array (hint_to_type_constant_list h)

and hints_to_type_constant l =
  TV.Array (
    List.mapi l
      ~f:(fun i h -> (TV.Int (Int64.of_int i), hint_to_type_constant h)))
