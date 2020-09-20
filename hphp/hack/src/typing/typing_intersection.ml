(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_core
open Typing_defs
module Env = Typing_env
module MkType = Typing_make_type
module Reason = Typing_reason
module SM = Nast.ShapeMap
module TySet = Typing_set
module Utils = Typing_utils

exception Nothing

(** Computes the negation of a type when it is known, which is currently the case
for null, nonnull, mixed, nothing, otherwise approximate up or down according to
`approx` parameter: If approx is `ApproxUp`, return mixed, else if it is `ApproxDown`,
return nothing. *)
let non env r ty ~approx =
  let (env, ty) = Env.expand_type env ty in
  let non_ty =
    match get_node ty with
    | Tprim Aast.Tnull -> MkType.nonnull r
    | Tnonnull -> MkType.null r
    | _ ->
      if Utils.equal_approx approx Utils.ApproxUp then
        MkType.mixed r
      else
        MkType.nothing r
  in
  (env, non_ty)

(** Decompose types as union of "atomic" elements.
In this context, "atomic" means: which can't be broken down in a union of smaller
types.
For now, only break Toption. We might also break down things like num or arraykey
in the future if necessary. Inverted by recompose_atomic. *)
let decompose_atomic env ty =
  let rec decompose_list tyl acc_tyl =
    match tyl with
    | [] -> acc_tyl
    | ty :: tyl -> decompose ty tyl acc_tyl
  and decompose ty tyl acc_tyl =
    match deref ty with
    | (r, Toption ty) -> decompose_list (ty :: tyl) (MkType.null r :: acc_tyl)
    | _ -> decompose_list tyl (ty :: acc_tyl)
  in
  let tyl = decompose ty [] [] in
  let tyl = TySet.elements (TySet.of_list tyl) in
  (env, MkType.union (get_reason ty) tyl)

(** Build a union from a list of types. Flatten if any of the types themselves are union.
    Also, pull nullable out to a top-level Toption. Undoes decompose atomic.
*)
let recompose_atomic env r tyl =
  let rec traverse nullable dynamic tyl_acc tyl =
    match tyl with
    | [] -> (nullable, dynamic, List.rev tyl_acc)
    | ty :: tyl ->
      if Utils.is_nothing env ty then
        traverse nullable dynamic tyl_acc tyl
      else (
        match deref ty with
        | (r, Toption ty) -> traverse (Some r) dynamic (ty :: tyl_acc) tyl
        | (r, Tprim Aast.Tnull) -> traverse (Some r) dynamic tyl_acc tyl
        | (r, Tdynamic) -> traverse nullable (Some r) tyl_acc tyl
        | (_, Tunion tyl') -> traverse nullable dynamic tyl_acc (tyl' @ tyl)
        | _ -> traverse nullable dynamic (ty :: tyl_acc) tyl
      )
  in
  let (nullable_r, dynamic_r, tyl) = traverse None None [] tyl in
  Utils.make_union env r tyl nullable_r dynamic_r

(** Number of '&' symbols in an intersection representation. E.g. for (A & B),
returns 1, for A, returns 0. *)
let number_of_inter_symbols env ty =
  let rec n_inter env tyl n =
    match tyl with
    | [] -> (env, n)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      begin
        match get_node ty with
        | Tintersection tyl' ->
          n_inter env (tyl' @ tyl) (List.length tyl' - 1 + n)
        | Tunion tyl' -> n_inter env (tyl' @ tyl) n
        | Toption ty -> n_inter env (ty :: tyl) n
        | _ -> n_inter env tyl n
      end
  in
  n_inter env [ty] 0

let collapses env ty1 ty2 ~inter_ty =
  let (env, n_inter_inter) = number_of_inter_symbols env inter_ty in
  let (env, n_inter1) = number_of_inter_symbols env ty1 in
  let (env, n_inter2) = number_of_inter_symbols env ty2 in
  (env, n_inter_inter <= n_inter1 + n_inter2)

let make_intersection env r tyl =
  let ty = MkType.intersection r tyl in
  let (env, ty) = Typing_utils.wrap_union_inter_ty_in_var env r ty in
  (env, ty)

(** Computes the intersection (greatest lower bound) of two types.
For the intersection of unions, attempt to simplify by using the distributivity
of intersection over union. Uses the the `collapses` test function to make sure
the resulting type is no greater in size than the trivial `Tintersection [ty1; ty2]`. *)
let rec intersect env ~r ty1 ty2 =
  let (env, ty1) = Env.expand_type env ty1 in
  let (env, ty2) = Env.expand_type env ty2 in
  if Utils.is_sub_type_for_union env ty1 ty2 then
    (env, ty1)
  else if Utils.is_sub_type_for_union env ty2 ty1 then
    (env, ty2)
  else
    let (env, non_ty2) = non env Reason.none ty2 Utils.ApproxDown in
    if Utils.is_sub_type_for_union env ty1 non_ty2 then
      (env, MkType.nothing r)
    else
      let (env, non_ty1) = non env Reason.none ty1 Utils.ApproxDown in
      if Utils.is_sub_type_for_union env ty2 non_ty1 then
        (env, MkType.nothing r)
      else
        let (env, ty1) = decompose_atomic env ty1 in
        let (env, ty2) = decompose_atomic env ty2 in
        let (env, inter_ty) =
          try
            match (deref ty1, deref ty2) with
            | ((_, Ttuple tyl1), (_, Ttuple tyl2))
              when Int.equal (List.length tyl1) (List.length tyl2) ->
              let (env, inter_tyl) =
                List.map2_env env tyl1 tyl2 ~f:(intersect ~r)
              in
              (env, mk (r, Ttuple inter_tyl))
            | ((_, Tshape (shape_kind1, fdm1)), (_, Tshape (shape_kind2, fdm2)))
              ->
              let (env, shape_kind, fdm) =
                intersect_shapes env r (shape_kind1, fdm1) (shape_kind2, fdm2)
              in
              (env, mk (r, Tshape (shape_kind, fdm)))
            | ((_, Tintersection tyl1), (_, Tintersection tyl2)) ->
              intersect_lists env r tyl1 tyl2
            | ((_, Tintersection tyl), _) -> intersect_lists env r [ty2] tyl
            | (_, (_, Tintersection tyl)) -> intersect_lists env r [ty1] tyl
            | ((r1, Tunion tyl1), (r2, Tunion tyl2)) ->
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
              let (env, not_common_tyl) =
                intersect_unions env r (r1, tyl1') (r2, tyl2')
              in
              recompose_atomic env r (common_tyl @ not_common_tyl)
            | ((r_union, Tunion tyl), ty)
            | (ty, (r_union, Tunion tyl)) ->
              let (env, inter_tyl) =
                intersect_ty_union env r (mk ty) (r_union, tyl)
              in
              recompose_atomic env r inter_tyl
            | _ -> make_intersection env r [ty1; ty2]
          with Nothing -> (env, MkType.nothing r)
        in
        Typing_log.log_intersection ~level:2 env r ty1 ty2 ~inter_ty;
        (env, inter_ty)

and intersect_shapes env r (shape_kind1, fdm1) (shape_kind2, fdm2) =
  let (env, fdm) =
    SM.merge_env env fdm1 fdm2 ~combine:(fun env _sfn sft1 sft2 ->
        match ((shape_kind1, sft1), (shape_kind2, sft2)) with
        | ((_, None), (_, None))
        | ((_, Some { sft_optional = true; _ }), (Closed_shape, None))
        | ((Closed_shape, None), (_, Some { sft_optional = true; _ })) ->
          (env, None)
        | ((_, Some { sft_optional = false; _ }), (Closed_shape, None))
        | ((Closed_shape, None), (_, Some { sft_optional = false; _ })) ->
          raise Nothing
        | ((_, Some sft), (Open_shape, None))
        | ((Open_shape, None), (_, Some sft)) ->
          (env, Some sft)
        | ( (_, Some { sft_optional = opt1; sft_ty = ty1 }),
            (_, Some { sft_optional = opt2; sft_ty = ty2 }) ) ->
          let opt = opt1 && opt2 in
          let (env, ty) = intersect env r ty1 ty2 in
          (env, Some { sft_optional = opt; sft_ty = ty }))
  in
  let shape_kind =
    match (shape_kind1, shape_kind2) with
    | (Open_shape, Open_shape) -> Open_shape
    | _ -> Closed_shape
  in
  (env, shape_kind, fdm)

and intersect_lists env r tyl1 tyl2 =
  let rec intersect_lists env tyl1 tyl2 acc_tyl =
    match (tyl1, tyl2) with
    | ([], _) -> (env, tyl2 @ acc_tyl)
    | (_, []) -> (env, tyl1 @ acc_tyl)
    | (ty1 :: tyl1', _) ->
      let (env, (inter_ty, missed_inter_tyl2)) =
        intersect_ty_tyl env r ty1 tyl2
      in
      intersect_lists env tyl1' missed_inter_tyl2 (inter_ty :: acc_tyl)
  in
  let (env, tyl) = intersect_lists env tyl1 tyl2 [] in
  make_intersection env r tyl

and intersect_ty_tyl env r ty tyl =
  let rec intersect_ty_tyl env ty tyl missed_inter_tyl =
    match tyl with
    | [] -> (env, (ty, missed_inter_tyl))
    | ty' :: tyl' ->
      let (env, ty_opt) = try_intersect env r ty ty' in
      begin
        match ty_opt with
        | None -> intersect_ty_tyl env ty tyl' (ty' :: missed_inter_tyl)
        | Some inter_ty -> intersect_ty_tyl env inter_ty tyl' missed_inter_tyl
      end
  in
  intersect_ty_tyl env ty tyl []

and try_intersect env r ty1 ty2 =
  let (env, ty) = intersect env r ty1 ty2 in
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tintersection _ -> (env, None)
  | _ -> (env, Some ty)

and intersect_unions env r (r1, tyl1) (r2, tyl2) =
  (* The order matters. (A | B | C) & (A | B) gets simplified to (A | B)
  while (A | B) & (A | B | C) would become A | B | ((A | B) & C), so we
  put the longest union first as a heuristic. *)
  let ((r1, tyl1), (r2, tyl2)) =
    if List.length tyl1 >= List.length tyl2 then
      ((r1, tyl1), (r2, tyl2))
    else
      ((r2, tyl2), (r1, tyl1))
  in
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
and intersect_ty_union env r (ty1 : locl_ty) (r_union, tyl2) =
  let (env, inter_tyl) =
    List.map_env env tyl2 ~f:(fun env ty2 -> intersect env r ty1 ty2)
  in
  let zipped = List.zip_exn tyl2 inter_tyl in
  let (collapsed, not_collapsed) =
    List.partition_tf zipped ~f:(fun (ty2, inter_ty) ->
        snd @@ collapses env ty1 ty2 ~inter_ty)
  in
  let collapsed = List.map collapsed ~f:snd in
  let not_collapsed =
    match not_collapsed with
    | [] -> []
    | [(_ty2, inter_ty)] -> [inter_ty]
    | _ ->
      let not_collapsed = List.map not_collapsed ~f:fst in
      [MkType.intersection r [ty1; MkType.union r_union not_collapsed]]
  in
  (env, not_collapsed @ collapsed)

let intersect_list env r tyl =
  (* We need to match tyl here because we'd mess the reason if tyl was just
  [mixed] *)
  match tyl with
  | [] -> (env, MkType.mixed r)
  | ty :: tyl -> List.fold_left_env env tyl ~init:ty ~f:(intersect ~r)

let normalize_intersection env ?on_tyvar tyl =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec normalize_intersection env r tyl tys_acc =
    match tyl with
    | [] -> (env, r, tys_acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      let proceed env ty =
        normalize_intersection env r tyl (TySet.add ty tys_acc)
      in
      begin
        match (deref ty, on_tyvar) with
        | ((r', Tvar v), Some on_tyvar) ->
          let (env, ty') = on_tyvar env r' v in
          if ty_equal ty ty' then
            proceed env ty
          else
            normalize_intersection env r (ty' :: tyl) tys_acc
        | ((r', Tintersection tyl'), _) ->
          normalize_intersection env (orr r r') (tyl' @ tyl) tys_acc
        | ((_, Tunion _), _) ->
          let (env, ty) = Utils.simplify_unions env ty ?on_tyvar in
          let (env, ety) = Env.expand_type env ty in
          begin
            match get_node ety with
            | Tunion _ -> proceed env ty
            | _ -> normalize_intersection env r (ty :: tyl) tys_acc
          end
        | _ -> proceed env ty
      end
  in
  normalize_intersection env None tyl TySet.empty

let simplify_intersections env ?on_tyvar ty =
  let r = get_reason ty in
  let (env, r', tys) = normalize_intersection env [ty] ?on_tyvar in
  let r = Option.value r' ~default:r in
  intersect_list env r (TySet.elements tys)

let intersect env ~r ty1 ty2 =
  let (env, inter_ty) = intersect env ~r ty1 ty2 in
  Typing_log.log_intersection ~level:1 env r ty1 ty2 ~inter_ty;
  (env, inter_ty)

let rec intersect_i env r ty1 lty2 =
  let ty2 = LoclType lty2 in
  if Utils.is_sub_type_for_union_i env ty1 ty2 then
    (env, ty1)
  else if Utils.is_sub_type_for_union_i env ty2 ty1 then
    (env, ty2)
  else
    let (env, ty) =
      match ty1 with
      | LoclType lty1 ->
        let (env, ty) = intersect env r lty1 lty2 in
        (env, LoclType ty)
      | ConstraintType cty1 ->
        (match deref_constraint_type cty1 with
        | (r, TCintersection (lty1, cty1)) ->
          let (env, lty) = intersect env r lty1 lty2 in
          ( env,
            ConstraintType (mk_constraint_type (r, TCintersection (lty, cty1)))
          )
        | (r, TCunion (lty1, cty1)) ->
          (* Distribute intersection over union.
          At the moment local types in TCunion can only be
          unions or intersections involving only null and nonnull,
          so applying distributivity allows for simplifying the types. *)
          let (env, lty) = intersect env r lty1 lty2 in
          let (env, ty) = intersect_i env r (ConstraintType cty1) lty2 in
          (match ty with
          | LoclType ty ->
            let (env, ty) = Utils.union env lty ty in
            (env, LoclType ty)
          | ConstraintType cty ->
            (env, ConstraintType (mk_constraint_type (r, TCunion (lty, cty)))))
        | (_, Thas_member _)
        | (_, Tdestructure _) ->
          ( env,
            ConstraintType (mk_constraint_type (r, TCintersection (lty2, cty1)))
          ))
    in
    match ty with
    | LoclType _ -> (env, ty)
    | ConstraintType ty -> Utils.simplify_constraint_type env ty

let () = Utils.non_ref := non

let () = Utils.simplify_intersections_ref := simplify_intersections
