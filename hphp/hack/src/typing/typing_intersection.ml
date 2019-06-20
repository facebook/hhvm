(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Env = Typing_env
module MkType = Typing_make_type
module Reason = Typing_reason
module TySet = Typing_set
module Utils = Typing_utils

let non env r ty ~approx =
  let (env, ty) = Env.expand_type env ty in
  let non_ty = match snd ty with
    | Tprim Nast.Tnull -> (r, Tnonnull)
    | Tnonnull -> (r, Tprim Nast.Tnull)
    | _ -> if approx = `up then MkType.mixed r else MkType.nothing r in
  env, non_ty

(** Decompose types as union of "atomic" elements.
In this context, "atomic" means: which can't be broken down in a union of smaller
types.
For now, only break Toption. We might also break down things like num or arraykey
in the future if necessary. *)
let decompose_atomic env ty =
  let rec decompose_list tyl acc_tyl =
    match tyl with
    | [] -> acc_tyl
    | ty :: tyl ->
      decompose ty tyl acc_tyl
  and decompose ty tyl acc_tyl =
    match ty with
    | (r, Toption ty) -> decompose_list (ty :: tyl) (MkType.null r :: acc_tyl)
    | _ -> decompose_list tyl (ty :: acc_tyl) in
  let tyl = decompose ty [] [] in
  let tyl = TySet.elements (TySet.of_list tyl) in
  env, MkType.union (fst ty) tyl

let rec intersect env r ty1 ty2 =
  let (env, ty1) = Env.expand_type env ty1 in
  let (env, ty2) = Env.expand_type env ty2 in
  if Utils.is_sub_type_for_union env ty1 ty2 then env, ty1 else
  if Utils.is_sub_type_for_union env ty2 ty1 then env, ty2 else
  let env, non_ty2 = non env Reason.none ty2 ~approx:`down in
  if Utils.is_sub_type_for_union env ty1 non_ty2 then env, MkType.nothing r else
  let env, non_ty1 = non env Reason.none ty1 ~approx:`down in
  if Utils.is_sub_type_for_union env ty2 non_ty1 then env, MkType.nothing r else
  let (env, ty1) = decompose_atomic env ty1 in
  let (env, ty2) = decompose_atomic env ty2 in
  match ty1, ty2 with
  | (_, Tintersection tyl1), (_, Tintersection tyl2) ->
    intersect_lists env r tyl1 tyl2
  | (_, Tintersection tyl), ty | ty, (_, Tintersection tyl) ->
    intersect_lists env r [ty] tyl
  | _ ->
    env, (r, Tintersection [ty1; ty2])

and intersect_lists env r tyl1 tyl2 =
  let rec intersect_lists env tyl1 tyl2 acc_tyl =
    match tyl1, tyl2 with
    | [], _ -> env, tyl2 @ acc_tyl
    | _, [] -> env, tyl1 @ acc_tyl
    | ty1 :: tyl1', _ ->
      let (env, (inter_ty, missed_inter_tyl2)) = intersect_ty_tyl env r ty1 tyl2 in
      intersect_lists env tyl1' missed_inter_tyl2 (inter_ty :: acc_tyl) in
  let env, tyl = intersect_lists env tyl1 tyl2 [] in
  env, MkType.intersection r tyl

and intersect_ty_tyl env r ty tyl =
  let rec intersect_ty_tyl env ty tyl missed_inter_tyl =
    match tyl with
    | [] -> env, (ty, missed_inter_tyl)
    | ty' :: tyl' ->
      let env, ty_opt = try_intersect env r ty ty' in
      begin match ty_opt with
      | None -> intersect_ty_tyl env ty tyl' (ty' :: missed_inter_tyl)
      | Some inter_ty -> intersect_ty_tyl env inter_ty tyl' missed_inter_tyl
      end in
  intersect_ty_tyl env ty tyl []

and try_intersect env r ty1 ty2 =
  let env, ty = intersect env r ty1 ty2 in
  match snd ty with
  | Tintersection _ -> env, None
  | _ -> env, Some ty

let intersect_list env r tyl =
  (* TODO T44713456 dummy implementation for now. *)
  env, (r, Tintersection tyl)
