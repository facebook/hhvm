(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

let singleton = ShapeKeyMap.singleton

let ( <> ) ~env sk1 sk2 =
  let merge _key ty_opt ty_opt' =
    match (ty_opt, ty_opt') with
    | (Some ty, Some ty') ->
      let (_env, ty) = Typing_union.union env ty ty' in
      Some ty
    | (None, (Some _ as ty_opt))
    | ((Some _ as ty_opt), None) ->
      ty_opt
    | (None, None) -> None
  in
  ShapeKeyMap.merge merge sk1 sk2
