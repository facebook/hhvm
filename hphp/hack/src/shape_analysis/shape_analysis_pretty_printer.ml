(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module A = Aast

let show_constraint_ env =
  let show_entity = function
    | Literal pos -> Format.asprintf "%a" Pos.pp pos
  in
  let show_key = function
    | A.String key -> key
    | _ -> failwith "Tried to print unsupported key"
  in
  let show_ty = Typing_print.full env in
  function
  | Exists entity -> "EX " ^ show_entity entity
  | Has_static_key (entity, key, ty) ->
    show_entity entity ^ " => " ^ show_key key ^ " : " ^ show_ty ty
  | Has_dynamic_key entity -> show_entity entity ^ " : dyn"
