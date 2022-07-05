(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

let singleton key ty optional = ShapeKeyMap.singleton key (ty, optional)

let ( <> ) ~env sk1 sk2 =
  let merge_optional_field = function
    | (FOptional, _) -> FOptional
    | (_, opt) -> opt
  in
  let merge_shape_key_map _key ty_opt ty_opt' =
    match (ty_opt, ty_opt') with
    | (Some (ty, opt), Some (ty', opt')) ->
      let (_env, ty'') = Typing_union.union env ty ty' in
      let opt'' = merge_optional_field (opt, opt') in
      Some (ty'', opt'')
    | (None, (Some _ as ty_opt))
    | ((Some _ as ty_opt), None) ->
      ty_opt
    | (None, None) -> None
  in
  let map = ShapeKeyMap.merge merge_shape_key_map sk1 sk2 in
  map
