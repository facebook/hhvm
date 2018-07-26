(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hh_core
open Hhbc_string_utils

module A = Ast
module TV = Typed_value

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

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
  | "array" -> 12
  | "typevar" | "hh\\_" -> 13 (* corresponds to user OF_GENERIC *)
  | "shape" -> 14
  | "class" -> 15
  | "interface" -> 16
  | "trait" -> 17
  | "enum" -> 18
  | "hh\\dict" -> 19
  | "hh\\vec" -> 20
  | "hh\\keyset" -> 21
  | "hh\\vec_or_dict" -> 22
  | "hh\\nonnull" -> 23
  | "hh\\darray" -> 24
  | "hh\\varray" -> 25
  | "hh\\varray_or_darray" ->  26
  | "hh\\arraylike" -> 27
  | "typeaccess" -> 102
  | _ when String.length p > 4 && String.sub p 0 4 = "xhp_" -> 103
  | "unresolved"
  | _ -> 101

and is_prim = function
  | "HH\\void" | "HH\\int"
  | "HH\\bool" | "HH\\float"
  | "HH\\string" | "HH\\resource"
  | "HH\\num" | "HH\\noreturn"
  | "HH\\arraykey" | "HH\\mixed"
  | "HH\\nonnull" -> true
  | _ -> false

and is_resolved_classname = function
  | "array"
  | "HH\\darray"
  | "HH\\varray"
  | "HH\\varray_or_darray"
  | "HH\\vec"
  | "HH\\dict"
  | "HH\\keyset"
  | "HH\\vec_or_dict"
  | "HH\\arraylike" -> true
  | _ -> false

let add_ns ~namespace id =
  let classname, _ = Hhbc_id.Class.elaborate_id namespace id in
  Hhbc_id.Class.to_raw_string classname

let check_shape_key (pos, name) =
  if String.length name > 0 && String_utils.is_decimal_digit name.[0]
  then Emit_fatal.raise_fatal_parse
    pos "Shape key names may not start with integers"

let shape_field_name ~namespace = function
  | A.SFlit_int (_, s) ->
    s, false
  | A.SFlit_str ((_, s) as id) ->
    check_shape_key id;
    s, false
  | A.SFclass_const (id, (_, s)) ->
    add_ns ~namespace id ^ "::" ^ s, true

let rec shape_field_to_pair ~tparams ~namespace sf =
  let name, is_class_const = shape_field_name ~namespace sf.A.sf_name in
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
  let value =
    if hack_arr_dv_arrs () then (TV.Dict inner_value) else (TV.DArray inner_value)
  in
    (TV.String name, value)

and shape_info_to_typed_value ~tparams ~namespace si =
  let info =
    List.map ~f:(shape_field_to_pair ~tparams ~namespace)
             si.A.si_shape_field_list
  in
  if hack_arr_dv_arrs () then (TV.Dict info) else (TV.DArray info)

and shape_allows_unknown_fields { A.si_allows_unknown_fields; _ } =
  if si_allows_unknown_fields then
    [TV.String "allows_unknown_fields", TV.Bool true]
  else []

and type_constant_access_list sl =
  let l = List.map ~f:(fun (_, s) -> TV.String s) sl
  in if hack_arr_dv_arrs () then (TV.Vec l) else (TV.VArray l)

and resolve_classname ~tparams ~namespace (p, s) =
  let s = add_ns namespace (p, Types.fix_casing s) in
  if is_prim s || is_resolved_classname s then [], s
  else
    let is_tparam = List.mem tparams s || (String.lowercase_ascii s) = "hh\\_" in
    let id = if is_tparam then "name" else "classname" in
    [TV.String id, TV.String s], s

and get_generic_types ~tparams ~namespace = function
  | [] -> []
  | l ->
    [TV.String "generic_types", hints_to_type_constant ~tparams ~namespace l]

and get_kind ~tparams s = [TV.String "kind", TV.Int (get_kind_num ~tparams s)]

and root_to_string ~namespace s =
  if s = "this" then prefix_namespace "HH" s
  else add_ns namespace (Pos.none, s)

and get_typevars = function
 | [] -> []
 | tparams -> [TV.String "typevars", TV.String (String.concat "," tparams)]

and hint_to_type_constant_list ~tparams ~namespace h =
  match snd h with
  | A.Happly (s, l) ->
    let classname, s_res = resolve_classname ~tparams ~namespace s in
    let kind = get_kind ~tparams s_res in
    let n = String.lowercase_ascii @@ snd s in
    let generic_types =
      if n = "classname" || n = "typename" then []
      else get_generic_types ~tparams ~namespace l in
    kind @ classname @ generic_types
  | A.Hshape (si) ->
    shape_allows_unknown_fields si
    @ get_kind ~tparams "shape"
    @ [TV.String "fields", shape_info_to_typed_value ~tparams ~namespace si]
  | A.Haccess ((_, s0), s1, sl) ->
    get_kind ~tparams "typeaccess" @
     [TV.String "root_name", TV.String (root_to_string ~namespace s0);
     TV.String "access_list", type_constant_access_list @@ s1::sl]
  | A.Hfun (true, _, _, _, _) ->
    failwith "Codegen for coroutine functions is not supported"
  | A.Hfun (false, hl, _kl, _b, h) ->
    (* TODO(mqian): Implement for inout parameters *)
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
  let tconsts = l @ if is_typedef then get_typevars tparams else [] in
  if hack_arr_dv_arrs () then (TV.Dict tconsts) else (TV.DArray tconsts)

and hints_to_type_constant ~tparams ~namespace l =
  let tconsts =
    List.map l
     ~f:(fun h -> hint_to_type_constant ~tparams ~namespace h)
  in
  if hack_arr_dv_arrs () then (TV.Vec tconsts) else (TV.VArray tconsts)
