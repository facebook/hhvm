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
  | Exists entity -> "EX " ^ show_entity entity
  | Has_static_key (entity, key, ty) ->
    "SK " ^ show_entity entity ^ " => " ^ show_key key ^ " : " ^ show_ty ty
  | Has_dynamic_key entity -> "DK " ^ show_entity entity ^ " : dyn"
  | Points_to (pointer_entity, pointed_entity) ->
    show_entity pointer_entity ^ " -> " ^ show_entity pointed_entity

let show_shape_result env = function
  | Shape_like_dict (pos, keys_and_types) ->
    let show_ty = show_ty env in
    let show_key_and_type (key, ty) =
      Format.asprintf "    %s => %s" (show_key key) (show_ty ty)
    in
    if List.is_empty keys_and_types then
      Format.asprintf "%s : shape()" (Format.asprintf "%a" Pos.pp pos)
    else
      Format.asprintf
        "%s :\n  shape(\n%s\n  )"
        (Format.asprintf "%a" Pos.pp pos)
        (String.concat ~sep:"\n" (List.map keys_and_types ~f:show_key_and_type))
  | Dynamically_accessed_dict entity ->
    Format.asprintf "%s : dynamic" (show_entity entity)
