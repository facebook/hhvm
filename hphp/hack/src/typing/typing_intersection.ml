(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_core
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

(** Number of '&' symbols in an intersection representation. E.g. for (A & B),
returns 1, for A, returns 0. *)
let number_of_inter_symbols ty =
  let rec n_inter tyl n =
    match tyl with
    | [] -> n
    | ty :: tyl ->
      begin match snd ty with
      | Tintersection tyl' -> n_inter (tyl' @ tyl) (List.length tyl' - 1 + n)
      | Tunion tyl' -> n_inter (tyl' @ tyl) n
      | Toption ty -> n_inter (ty :: tyl) n
      | _ -> n_inter tyl n
      end in
  n_inter [ty] 0

let collapses ty1 ty2 ~inter_ty =
  let n_inter_inter = number_of_inter_symbols inter_ty and
      n_inter1      = number_of_inter_symbols ty1 and
      n_inter2      = number_of_inter_symbols ty2 in
  n_inter_inter <= n_inter1 + n_inter2

(** Computes the intersection (greatest lower bound) of two types.
For the intersection of unions, attempt to simplify by using the distributivity
of intersection over union. Uses the the `collapses` test function to make sure
the resulting type is no greater in size than the trivial `Tintersection [ty1; ty2]`. *)
let rec intersect env ~r ty1 ty2 =
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
  let env, inter_ty = match ty1, ty2 with
    | (_, Tintersection tyl1), (_, Tintersection tyl2) ->
      intersect_lists env r tyl1 tyl2
    | (_, Tintersection tyl), ty | ty, (_, Tintersection tyl) ->
      intersect_lists env r [ty] tyl
    | (r1, Tunion tyl1), (r2, Tunion tyl2) ->
      (* Factorize common types, for example
         (A | B) & (A | C) = A | (B & C)
      and
         (A | B | C1 | D1) & (A | B | C2 | D2) = A | B | ((C1 | D1) & (C2 | D2))
      *)
      let tys1 = TySet.of_list tyl1 in
      let tys2 = TySet.of_list tyl2 in
      let common_tys = TySet.inter tys1 tys2 in
      let tys1' = TySet.diff tys1 common_tys in
      let tys2' = TySet.diff tys2 common_tys in
      let tyl1' = TySet.elements tys1' in
      let tyl2' = TySet.elements tys2' in
      let common_tyl = TySet.elements common_tys in
      let env, not_common_tyl = intersect_unions env r (r1, tyl1') (r2, tyl2') in
      Utils.fold_union env r (common_tyl @ not_common_tyl)
    | (r_union, Tunion tyl), ty | ty, (r_union, Tunion tyl) ->
      let env, inter_tyl = intersect_ty_union env r ty (r_union, tyl) in
      Utils.fold_union env r inter_tyl
    | _ ->
      env, (r, Tintersection [ty1; ty2]) in
  Typing_log.log_intersection ~level:2 env r ty1 ty2 ~inter_ty;
  env, inter_ty

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

and intersect_unions env r (r1, tyl1) (r2, tyl2) =
  (* The order matters. (A | B | C) & (A | B) gets simplified to (A | B)
  while (A | B) & (A | B | C) would become A | B | ((A | B) & C), so we
  put the longest union first as a heuristic. *)
  let (r1, tyl1), (r2, tyl2) = if List.length tyl1 >= List.length tyl2
    then (r1, tyl1), (r2, tyl2) else (r2, tyl2), (r1, tyl1) in
  let union_ty1 = MkType.union r1 tyl1 in
  intersect_ty_union env r union_ty1 (r2, tyl2)

(** For (A1 | .. | An) & B, compute each of the A1 & B, .. , An & B.
Keep those which collapse (see `collapses` function) with B
and leave the others unchanged.
So if I is the set of indices i such that Ai collapses with B,
and J the set of indices such that this is not the case,
the result would be
  (|_{i in I} (Ai & B)) | (B & (|_{j in J} Aj))
*)
and intersect_ty_union env r ty1 (r_union, tyl2) =
  let env, inter_tyl = List.map_env env tyl2 ~f:(fun env ty2 -> intersect env r ty1 ty2) in
  let zipped = List.zip_exn tyl2 inter_tyl in
  let (collapsed, not_collapsed) = List.partition_tf zipped
    ~f:(fun (ty2, inter_ty) -> collapses ty1 ty2 ~inter_ty) in
  let collapsed = List.map collapsed ~f:snd in
  let not_collapsed = match not_collapsed with
    | [] -> []
    | [(_ty2,  inter_ty)] -> [inter_ty]
    | _ ->
      let not_collapsed = List.map not_collapsed ~f:fst in
      [MkType.intersection r [ty1; (MkType.union r_union not_collapsed)]] in
  env, not_collapsed @ collapsed

let intersect_list env r tyl =
  List.fold_left_env env tyl ~init:(MkType.mixed r) ~f:(intersect ~r)


let normalize_intersection env ?on_tyvar tyl =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec normalize_intersection env r tyl tys_acc =
    match tyl with
    | [] -> env, r, tys_acc
    | ty :: tyl ->
      let env, ty = Env.expand_type env ty in
      let proceed env ty =
        normalize_intersection env r tyl (TySet.add ty tys_acc) in
      begin match ty, on_tyvar with
      | (r', Tvar v), Some on_tyvar ->
        let env, ty' = on_tyvar env r' v in
        if ty_equal ty ty' then proceed env ty else
        normalize_intersection env r (ty' :: tyl) tys_acc
      | (r', Tintersection tyl'), _ ->
        normalize_intersection env (orr r r') (tyl' @ tyl) tys_acc
      | (_, Tunion _), _ ->
        let env, ty = Utils.simplify_unions env ty ?on_tyvar in
        begin match ty with
        | (_, Tunion _) -> proceed env ty
        | _ -> normalize_intersection env r (ty :: tyl) tys_acc
        end
      | _ ->
        proceed env ty
      end in
  normalize_intersection env None tyl TySet.empty

let simplify_intersections env ?on_tyvar (r, _ as ty) =
  let env, r', tys = normalize_intersection env [ty] ?on_tyvar in
  let r = Option.value r' ~default:r in
  intersect_list env r (TySet.elements tys)

let intersect env ~r ty1 ty2 =
  let env, inter_ty = intersect env ~r ty1 ty2 in
  Typing_log.log_intersection ~level:1 env r ty1 ty2 ~inter_ty;
  env, inter_ty

let () = Utils.simplify_intersections_ref := simplify_intersections
