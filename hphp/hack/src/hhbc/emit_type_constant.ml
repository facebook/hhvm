(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Hhbc_string_utils

module A = Ast
module TV = Typed_value

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

(* Taken from: hphp/runtime/base/type-structure.h *)
let get_kind_num ~tparams p =
  let p = if List.mem ~equal:(=) tparams p then "typevar" else String.lowercase p in
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
  | "typevar" | "_" -> 13 (* corresponds to user OF_GENERIC *)
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
  | "hh\\null" -> 28
  | "hh\\nothing" -> 29
  | "hh\\dynamic" -> 30
  | "typeaccess" -> 102
  | _ when String.length p > 4 && String.sub p 0 4 = "xhp_" -> 103
  | "reifiedtype" -> 104
  | "unresolved"
  | _ -> 101

and is_prim = function
  | "HH\\void" | "HH\\int"
  | "HH\\bool" | "HH\\float"
  | "HH\\string" | "HH\\resource"
  | "HH\\num" | "HH\\noreturn"
  | "HH\\arraykey" | "HH\\mixed"
  | "HH\\nonnull" | "HH\\null"
  | "HH\\nothing" | "HH\\dynamic" -> true
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
  let classname = Hhbc_id.Class.elaborate_id namespace id in
  Hhbc_id.Class.to_raw_string classname

let shape_field_name ~namespace = function
  | A.SFlit_int (_, s)
  | A.SFlit_str (_, s) ->
    s, false
  | A.SFclass_const (id, (_, s)) ->
    add_ns ~namespace id ^ "::" ^ s, true

let rec shape_field_to_pair ~tparams ~namespace ~targ_map sfi =
  let name, is_class_const = shape_field_name ~namespace sfi.Aast.sfi_name in
  let is_optional = sfi.Aast.sfi_optional in
  let hint = sfi.Aast.sfi_hint in
  let class_const =
    if is_class_const
    then [(TV.String "is_cls_cns", TV.Bool true)]
    else [] in
  let optional =
    if is_optional
    then [(TV.String "optional_shape_field", TV.Bool true)]
    else [] in
  let inner_value =
    [(TV.String "value",
      hint_to_type_constant ~tparams ~namespace ~targ_map hint)] in
  let inner_value = class_const @ optional @ inner_value in
  let value =
    if hack_arr_dv_arrs ()
    then TV.Dict inner_value
    else TV.DArray inner_value in
  (TV.String name, value)

and shape_info_to_typed_value ~tparams ~namespace ~targ_map (si : Aast.nast_shape_info): TV.t =
  let info =
    List.map
      ~f:(shape_field_to_pair ~tparams ~namespace ~targ_map)
      si.Aast.nsi_field_map in
  if hack_arr_dv_arrs ()
  then TV.Dict info
  else TV.DArray info

and shape_allows_unknown_fields { Aast.nsi_allows_unknown_fields; _ } =
  if nsi_allows_unknown_fields then
    [TV.String "allows_unknown_fields", TV.Bool true]
  else []

and type_constant_access_list sl =
  let l = List.map ~f:(fun (_, s) -> TV.String s) sl in
  if hack_arr_dv_arrs ()
  then TV.Vec l
  else TV.VArray l

and resolve_classname ~tparams ~namespace (p, s) =
  let is_tparam = s = "_" || List.mem ~equal:(=) tparams s in
  let s = if is_tparam then s else add_ns namespace (p, Types.fix_casing s) in
  if is_prim s || is_resolved_classname s then [], s
  else
    let id = if is_tparam then "name" else "classname" in
    [TV.String id, TV.String s], s

and get_generic_types ~tparams ~namespace ~targ_map hl =
  match hl with
  | [] -> []
  | _ ->
    (TV.String "generic_types",
      hints_to_type_constant ~tparams ~namespace ~targ_map hl) :: []

and get_kind ~tparams s = [TV.String "kind", TV.Int (get_kind_num ~tparams s)]

and root_to_string ~namespace s =
  if s = "this" then prefix_namespace "HH" s
  else add_ns namespace (Pos.none, s)

and get_typevars = function
 | [] -> []
 | tparams -> [TV.String "typevars", TV.String (String.concat ~sep:"," tparams)]

and hint_to_type_constant_list ~tparams ~namespace ~targ_map (h : Aast.hint) =
  match snd h with
  | Aast.Happly ((_, name), []) when SMap.mem name targ_map ->
    let id =
      match SMap.get name targ_map with
      | Some i -> Int64.of_int i
      | None -> failwith "impossible" in
    get_kind ~tparams "reifiedtype" @ [TV.String "id", TV.Int id]
  | Aast.Happly (s, l) ->
    let classname, s_res = resolve_classname ~tparams ~namespace s in
    let kind = get_kind ~tparams s_res in
    let n = String.lowercase @@ snd s in
    let generic_types =
      if n = "classname" || n = "typename"
      then []
      else get_generic_types ~tparams ~namespace ~targ_map l in
    kind @ classname @ generic_types
  | Aast.Hshape (si) ->
    shape_allows_unknown_fields si @
      get_kind ~tparams "shape" @
      [ (TV.String "fields",
        shape_info_to_typed_value ~tparams ~namespace ~targ_map si) ]
  (* Matches the structure in ast_to_nast.ml on_hint for Haccess *)
  | Aast.Haccess ((_, Aast.Happly ((_, root_id), [])), ids) ->
    get_kind ~tparams "typeaccess" @
      [ TV.String "root_name", TV.String (root_to_string ~namespace root_id);
        TV.String "access_list", type_constant_access_list ids
      ]
  | Aast.Haccess _ -> failwith "Structure not translated according to ast_to_nast"
  | Aast.Hfun (_, true (* is_coroutine *), _, _, _, _, _, _ ) ->
    failwith "Codegen for coroutine functions is not supported"
  | Aast.Hfun (_, false (* is_coroutine *), hl, _kl, _, _b, h, _) ->
    (* TODO(mqian): Implement for inout parameters *)
    (* TODO: Bool indicates variadic argument. What to do? *)
    let kind = get_kind ~tparams "fun" in
    let return_type =
      [(TV.String "return_type"
        , hint_to_type_constant ~tparams ~namespace ~targ_map h)] in
    let param_types =
      [(TV.String "param_types"
        , hints_to_type_constant ~tparams ~namespace ~targ_map hl)] in
    kind @ return_type @ param_types
  | Aast.Hoption h ->
    [(TV.String "nullable", TV.Bool true)] @
      hint_to_type_constant_list ~tparams ~namespace ~targ_map h
  | Aast.Htuple hl ->
    let kind = get_kind ~tparams "tuple" in
    let elem_types =
      [(TV.String "elem_types"
        , hints_to_type_constant ~tparams ~namespace ~targ_map hl)] in
    kind @ elem_types
  | Aast.Hsoft h ->
    [TV.String "soft", TV.Bool true]
    @ hint_to_type_constant_list ~tparams ~namespace ~targ_map h
  | Aast.Hlike h ->
    [TV.String "like", TV.Bool true]
    @ hint_to_type_constant_list ~tparams ~namespace ~targ_map h
  (* TAST hints not found on the legacy AST *)
  | Aast.Hany
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hprim _
  | Aast.Hthis
  | Aast.Hnothing
  | Aast.Hdynamic -> failwith "Hints not available on the original AST"

and hint_to_type_constant ?(is_typedef = false) ?(is_opaque = false)
  ~tparams ~namespace ~targ_map (h : Aast.hint) =
  let l = hint_to_type_constant_list ~tparams ~namespace ~targ_map h in
  let tconsts =
    if is_typedef then l @ get_typevars tparams else l in
  let tconsts =
    if is_opaque then tconsts @ [TV.String "opaque", TV.Bool true] else tconsts
  in
  if hack_arr_dv_arrs ()
  then TV.Dict tconsts
  else TV.DArray tconsts

and hints_to_type_constant ~tparams ~namespace ~targ_map hl =
  let tconsts =
    List.map
      ~f:(fun h -> hint_to_type_constant ~tparams ~namespace ~targ_map h)
      hl in
  if hack_arr_dv_arrs ()
  then TV.Vec tconsts
  else TV.VArray tconsts
