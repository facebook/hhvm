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
