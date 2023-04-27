(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_defs

let rec wipe ty =
  let wp node = mk (Reason.Rnone, node) in
  let wtlist tlist = List.map ~f:wipe tlist in
  match get_node ty with
  | Toption ty -> wp (Toption (wipe ty))
  | Ttuple tlist ->
    let wlist = wtlist tlist in
    wp (Ttuple wlist)
  | Tunion tlist ->
    let wlist = wtlist tlist in
    wp (Tunion wlist)
  | Tintersection tlist ->
    let wlist = wtlist tlist in
    wp (Tintersection wlist)
  | Tgeneric (s, tlist) ->
    let wlist = wtlist tlist in
    wp (Tgeneric (s, wlist))
  | Tvec_or_dict (t1, t2) ->
    let w1 = wipe t1 in
    let w2 = wipe t2 in
    wp (Tvec_or_dict (w1, w2))
  | Tnewtype (s, tlist, ty) ->
    let wlist = wtlist tlist in
    let w = wipe ty in
    wp (Tnewtype (s, wlist, w))
  | Tdependent (d, ty) ->
    let w = wipe ty in
    wp (Tdependent (d, w))
  | Tclass (p, _, tlist) ->
    let wlist = wtlist tlist in
    (*exact contains reason, so we wipe it out*)
    wp (Tclass (p, Typing_defs_core.Exact, wlist))
  | Taccess (ty, pos_id) ->
    let w = wipe ty in
    wp (Taccess (w, pos_id))
  | Tshape (_, kind, map) ->
    let map =
      TShapeMap.map (fun sft -> { sft with sft_ty = wipe sft.sft_ty }) map
    in
    wp (Tshape (Missing_origin, kind, map))
  | Tfun ft ->
    let wt_et et = { et with et_type = wipe et.et_type } in
    let wt_fp fp = { fp with fp_type = wt_et fp.fp_type } in
    let wt_wc (lb, kind, ub) = (wipe lb, kind, wipe ub) in
    let rec wt_tp tp =
      let wt_c (kind, ty) = (kind, wipe ty) in
      {
        tp with
        tp_tparams = List.map ~f:wt_tp tp.tp_tparams;
        tp_constraints = List.map ~f:wt_c tp.tp_constraints;
      }
    in
    let wt_ip { capability } =
      let wt_cap cap =
        match cap with
        | CapTy ty -> CapTy (wipe ty)
        | CapDefaults _ -> cap
      in
      { capability = wt_cap capability }
    in
    let ft =
      {
        ft with
        ft_ret = wt_et ft.ft_ret;
        ft_params = List.map ~f:wt_fp ft.ft_params;
        ft_where_constraints = List.map ~f:wt_wc ft.ft_where_constraints;
        ft_tparams = List.map ~f:wt_tp ft.ft_tparams;
        ft_implicit_params = wt_ip ft.ft_implicit_params;
      }
    in
    wp (Tfun ft)
    (*we just wipe out the reason because below types are not recursive.*)
  | Tany _
  | Tnonnull
  | Tdynamic
  | Tprim _
  | Tvar _
  | Tunapplied_alias _
  | Tneg _ ->
    Typing_defs_core.(with_reason ty Reason.Rnone)
