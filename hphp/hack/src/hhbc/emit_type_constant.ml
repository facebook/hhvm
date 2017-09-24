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
  | "hh\\void" -> 0
  | "hh\\int" -> 1
  | "hh\\bool" -> 2
  | "hh\\float" -> 3
  | "hh\\string" -> 4
  | "hh\\resource" -> 5
  | "hh\\num" -> 6
  | "hh\\noreturn" -> 8
  | "hh\\arraykey" -> 7
  | "hh\\mixed" -> 9
  | "tuple" -> 10
  | "fun" -> 11
  | "hh\\darray" | "hh\\varray" | "array" -> 12
  | "typevar" -> 13 (* corresponds to user OF_GENERIC *)
  | "shape" -> 14
  | "class" -> 15
  | "interface" -> 16
  | "trait" -> 17
  | "enum" -> 18
  | "hh\\dict" -> 19
  | "hh\\vec" -> 20
  | "hh\\keyset" -> 21
  | "typeaccess" -> 102
  | "xhp" -> 103
  | "unresolved"
  | _ -> 101

and is_prim = function
  | "HH\\void" | "HH\\int"
  | "HH\\bool" | "HH\\float"
  | "HH\\string" | "HH\\resource"
  | "HH\\num" | "HH\\noreturn"
  | "HH\\arraykey" | "HH\\mixed" -> true
  | _ -> false

and is_resolved_classname = function
  | "HH\\darray" | "HH\\varray"
  | "array" | "HH\\vec"
  | "HH\\dict" | "HH\\keyset" -> true
  | _ -> false

let shape_field_name = function
  | A.SFlit ((_, s)) -> s, false
  | A.SFclass_const ((_, id), (_, s)) -> id ^ "::" ^ s, true

let rec shape_field_to_pair ~tparams ~namespace sf =
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
    [(TV.String "value", hint_to_type_constant ~tparams ~namespace hint)]
  in
  let inner_value = class_const @ optional @ inner_value in
  let value = TV.Array inner_value in
  (TV.String name, value)

and shape_info_to_typed_value ~tparams ~namespace si =
  TV.Array (
    List.map ~f:(shape_field_to_pair ~tparams ~namespace)
    si.A.si_shape_field_list)

and shape_allows_unknown_fields { A.si_allows_unknown_fields; _ } =
  if si_allows_unknown_fields then
    [TV.String "allows_unknown_fields", TV.Bool true]
  else []

and type_constant_access_list sl =
  let l =
    List.mapi ~f:(fun i (_, s) -> (TV.Int (Int64.of_int i), TV.String s)) sl
  in
  TV.Array l

and resolve_classname ~tparams ~namespace (p, s) =
  let s = Types.fix_casing s in
  let classname, _ = Hhbc_id.Class.elaborate_id namespace (p, s) in
  let s = Hhbc_id.Class.to_raw_string classname in
  if is_prim s || is_resolved_classname s then [], s
  else
    let id = if List.mem tparams s then "name" else "classname" in
    [TV.String id, TV.String s], s

and get_generic_types ~tparams ~namespace = function
  | [] -> []
  | l ->
    [TV.String "generic_types", hints_to_type_constant ~tparams ~namespace l]

and get_kind ~tparams s = [TV.String "kind", TV.Int (get_kind_num ~tparams s)]

and root_to_string s =
  if s = "this" then prefix_namespace "HH" s
  else s

and get_typevars = function
 | [] -> []
 | tparams -> [TV.String "typevars", TV.String (String.concat "," tparams)]

and hint_to_type_constant_list ~tparams ~namespace h =
  match snd h with
  | A.Happly (s, l) ->
    let classname, s_res = resolve_classname ~tparams ~namespace s in
    let kind = get_kind ~tparams s_res in
    let generic_types =
      if snd s = "classname" then []
      else get_generic_types ~tparams ~namespace l in
    kind @ classname @ generic_types
  | A.Hshape (si) ->
    shape_allows_unknown_fields si
    @ get_kind ~tparams "shape"
    @ [TV.String "fields", shape_info_to_typed_value ~tparams ~namespace si]
  | A.Haccess ((_, s0), s1, sl) ->
    get_kind ~tparams "typeaccess" @
     [TV.String "root_name", TV.String (root_to_string s0);
     TV.String "access_list", type_constant_access_list @@ s1::sl]
  | A.Hfun (true, _, _, _) ->
    failwith "Codegen for coroutine functions is not supported"
  | A.Hfun (false, hl, _b, h) ->
    (* TODO: Bool indicates variadic argument. What to do? *)
    let kind = get_kind ~tparams "fun" in
    let return_type =
      [TV.String "return_type", hint_to_type_constant ~tparams ~namespace h]
    in
    let param_types =
      [TV.String "param_types", hints_to_type_constant ~tparams ~namespace hl]
    in
    kind @ return_type @ param_types
  | A.Hoption h ->
    [TV.String "nullable", TV.Bool true]
    @ hint_to_type_constant_list ~tparams ~namespace h
  | A.Htuple hl ->
    let kind = get_kind ~tparams "tuple" in
    let elem_types =
      [TV.String "elem_types", hints_to_type_constant ~tparams ~namespace hl]
    in
    kind @ elem_types
  | A.Hsoft h -> hint_to_type_constant_list ~tparams ~namespace h

and hint_to_type_constant ?(is_typedef = false) ~tparams ~namespace h =
  let l = hint_to_type_constant_list ~tparams ~namespace h in
  TV.Array (l @ if is_typedef then get_typevars tparams else [])

and hints_to_type_constant ~tparams ~namespace l =
  TV.Array (
    List.mapi l
      ~f:(fun i h ->
            (TV.Int (Int64.of_int i),
            hint_to_type_constant ~tparams ~namespace h)))
