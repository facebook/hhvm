(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Typing_defs
open Shape_analysis_types

let singleton key sft_ty sft_optional =
  T.TShapeMap.singleton key T.{ sft_ty; sft_optional }

let ( <> ) ~env sk1 sk2 =
  let merge_shape_key_map _key ty_opt ty_opt' =
    match (ty_opt, ty_opt') with
    | (Some sft, Some sft') ->
      let (_env, sft_ty) = Typing_union.union env sft.T.sft_ty sft'.T.sft_ty in
      let sft_optional = sft.T.sft_optional && sft'.T.sft_optional in
      Some T.{ sft_ty; sft_optional }
    | (None, (Some _ as ty_opt))
    | ((Some _ as ty_opt), None) ->
      ty_opt
    | (None, None) -> None
  in
  T.TShapeMap.merge merge_shape_key_map sk1 sk2
