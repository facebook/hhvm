(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

let result_id_counter = ref 0

let fresh_result_id () =
  let result_id = !result_id_counter in
  result_id_counter := result_id + 1;
  ResultID.singleton result_id

let singleton result_id key ty = (result_id, ShapeKeyMap.singleton key ty)

let ( <> ) ~env (id1, sk1) (id2, sk2) =
  let merge_shape_key_map _key ty_opt ty_opt' =
    match (ty_opt, ty_opt') with
    | (Some ty, Some ty') ->
      let (_env, ty) = Typing_union.union env ty ty' in
      Some ty
    | (None, (Some _ as ty_opt))
    | ((Some _ as ty_opt), None) ->
      ty_opt
    | (None, None) -> None
  in
  let result_id = ResultID.union id1 id2 in
  let map = ShapeKeyMap.merge merge_shape_key_map sk1 sk2 in
  (result_id, map)
