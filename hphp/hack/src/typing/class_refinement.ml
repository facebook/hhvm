(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

let is_empty { cr_consts } = SMap.is_empty cr_consts

let has_refined_const pos_id cr = SMap.mem (snd pos_id) cr.cr_consts

let get_refined_const (_, id) cr = SMap.find_opt id cr.cr_consts

let fold_refined_consts cr ~init:acc ~f = SMap.fold f cr.cr_consts acc

let add_refined_const id tr cr =
  let combine (type a) (r1 : a refined_const) (r2 : a refined_const) =
    (* Note: previous stages ensure that is_ctx are compatible *)
    let rc_bound =
      match (r1.rc_bound, r2.rc_bound) with
      | ((TRexact _ as exact), _)
      | (_, (TRexact _ as exact)) ->
        exact
      | ( TRloose { tr_lower = ls1; tr_upper = us1 },
          TRloose { tr_lower = ls2; tr_upper = us2 } ) ->
        TRloose { tr_lower = ls1 @ ls2; tr_upper = us1 @ us2 }
    in
    { rc_bound; rc_is_ctx = r1.rc_is_ctx }
  in
  { cr_consts = SMap.add ~combine id tr cr.cr_consts }

let map_refined_const (type a) f ({ rc_bound; rc_is_ctx } : a refined_const) =
  let rc_bound =
    match rc_bound with
    | TRexact ty -> TRexact (f ty)
    | TRloose { tr_lower = ls; tr_upper = us } ->
      TRloose { tr_lower = List.map ls ~f; tr_upper = List.map us ~f }
  in
  { rc_bound; rc_is_ctx }

let map f { cr_consts = rcs } =
  { cr_consts = SMap.map (map_refined_const f) rcs }

let fold_map_refined_const
    (type ph acc)
    (f : acc -> ph ty -> acc * ph ty)
    (acc : acc)
    _
    ({ rc_bound; rc_is_ctx } : ph refined_const) =
  let (acc, rc_bound) =
    match rc_bound with
    | TRexact ty ->
      let (acc, ty) = f acc ty in
      (acc, TRexact ty)
    | TRloose { tr_lower = ls; tr_upper = us } ->
      let (acc, ls) = List.fold_map ~f ~init:acc ls in
      let (acc, us) = List.fold_map ~f ~init:acc us in
      (acc, TRloose { tr_lower = ls; tr_upper = us })
  in
  (acc, { rc_bound; rc_is_ctx })

let fold_map (f : 'acc -> decl_ty -> 'acc * decl_ty) acc { cr_consts = rcs } :
    'acc * decl_phase class_refinement =
  let (acc, rcs) = SMap.map_env (fold_map_refined_const f) acc rcs in
  (acc, { cr_consts = rcs })

let fold_refined_const { rc_bound; rc_is_ctx = _ } ~init:acc ~f =
  match rc_bound with
  | TRexact ty -> f acc ty
  | TRloose { tr_lower = ls; tr_upper = us } ->
    let acc = List.fold ~f ~init:acc ls in
    let acc = List.fold ~f ~init:acc us in
    acc

let fold { cr_consts = rcs } ~init:acc ~f =
  SMap.fold (fun _ rc acc -> fold_refined_const rc ~init:acc ~f) rcs acc

let iter f r = fold r ~init:() ~f:(fun () -> f)

let to_string ty_to_string cr =
  let rc_to_string { rc_bound; _ } =
    match rc_bound with
    | TRexact ty -> "= " ^ ty_to_string ty
    | TRloose { tr_lower = ls; tr_upper = us } ->
      let l1 = List.map ls ~f:(fun ty -> "as " ^ ty_to_string ty) in
      let l2 = List.map us ~f:(fun ty -> "super " ^ ty_to_string ty) in
      String.concat ~sep:" " (l1 @ l2)
  in
  let members_list =
    let f name rc acc =
      let kind = refined_const_kind_str rc in
      (kind ^ " " ^ name ^ " " ^ rc_to_string rc) :: acc
    in
    SMap.fold f cr.cr_consts []
  in
  "{" ^ String.concat ~sep:"; " members_list ^ "}"
