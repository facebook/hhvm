(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types

let show_entity = function
  | Literal pos -> Format.asprintf "%a" Pos.pp pos
  | Variable var -> Format.sprintf "?%d" var

let show_key = function
  | SK_string key -> key

let show_ty env = Typing_print.full env

let show_constraint_ env =
  let show_ty = show_ty env in
  function
  | Exists (Allocation, pos) -> Format.asprintf "Allocated at %a" Pos.pp pos
  | Exists (Parameter, pos) -> Format.asprintf "Parameter at %a" Pos.pp pos
  | Exists (Argument, pos) -> Format.asprintf "Argument at %a" Pos.pp pos
  | Has_static_keys (entity, (result_id, shape_keys)) ->
    let static_keys =
      ShapeKeyMap.bindings shape_keys
      |> List.map ~f:(fun (key, ty) ->
             Format.asprintf "%s => %s" (show_key key) (show_ty ty))
      |> String.concat ~sep:", "
    in
    let result_id_str =
      ResultID.elements result_id
      |> List.map ~f:string_of_int
      |> String.concat ~sep:","
      |> Format.asprintf "{%s}"
    in
    Format.asprintf
      "SK #%s %s : shape(%s)"
      result_id_str
      (show_entity entity)
      static_keys
  | Has_dynamic_key entity -> "DK " ^ show_entity entity ^ " : dyn"
  | Subset (sub, sup) -> show_entity sub ^ " ⊆ " ^ show_entity sup

let show_shape_result env = function
  | Shape_like_dict (pos, result_id, keys_and_types) ->
    let result_id_str =
      ResultID.elements result_id
      |> List.map ~f:string_of_int
      |> String.concat ~sep:","
      |> Format.asprintf "{%s}"
    in
    let show_ty = show_ty env in
    let show_key_and_type (key, ty) =
      Format.asprintf "    %s => %s" (show_key key) (show_ty ty)
    in
    if List.is_empty keys_and_types then
      Format.asprintf
        "#%s %s : shape()"
        result_id_str
        (Format.asprintf "%a" Pos.pp pos)
    else
      Format.asprintf
        "#%s %s :\n  shape(\n%s\n  )"
        result_id_str
        (Format.asprintf "%a" Pos.pp pos)
        (String.concat ~sep:"\n" (List.map keys_and_types ~f:show_key_and_type))
  | Dynamically_accessed_dict entity ->
    Format.asprintf "%s : dynamic" (show_entity entity)
