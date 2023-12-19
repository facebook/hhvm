(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Env = Typing_env
module ITySet = Internal_type_set
module MakeType = Typing_make_type
module TUtils = Typing_utils
module TySet = Typing_set

let filter_locl_types types =
  ITySet.fold
    (fun ty types ->
      match ty with
      | LoclType ty -> TySet.add ty types
      | _ -> types)
    types
    TySet.empty

(** If a type variable appear in one of its own lower bounds under a combination
    of unions and intersections, it can be simplified away from this lower bound by
    replacing any of its occurences with nothing.
    E.g.
    - if #1 has lower bound (#1 | A), the lower bound can be simplified to
    (nothing | A) = A.
    - if #1 has lower bound (#1 & A), the lower bound can be simplified to
    (nothing & A) = nothing.
    *)
let remove_tyvar_from_lower_bound env var lower_bound =
  let is_nothing = ty_equal (MakeType.nothing Reason.none) in
  let rec remove env ty =
    let (env, ty) = Env.expand_type env ty in
    match deref ty with
    | (_, Tvar v) when Tvid.equal v var -> (env, MakeType.nothing Reason.none)
    | (r, Toption ty) ->
      let (env, ty) = remove env ty in
      (env, MakeType.nullable r ty)
    | (r, Tunion tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let tyl = List.filter tyl ~f:(fun ty -> not (is_nothing ty)) in
      (env, MakeType.union r tyl)
    | (r, Tintersection tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let ty =
        if List.exists tyl ~f:is_nothing then
          MakeType.nothing r
        else
          MakeType.intersection r tyl
      in
      (env, ty)
    | (r, Tnewtype (name, [tyarg], _))
      when String.equal name Naming_special_names.Classes.cSupportDyn ->
      let (env, ty) = remove env tyarg in
      if is_nothing ty then
        (env, MakeType.nothing r)
      else
        (env, MakeType.supportdyn r ty)
    | _ -> (env, ty)
  and remove_i env ty =
    match ty with
    | LoclType ty ->
      let (env, ty) = remove env ty in
      (env, LoclType ty)
    | _ -> (env, ty)
  in
  remove_i env lower_bound

let remove_tyvar_from_lower_bounds env var lower_bounds =
  let (env, lower_bounds) =
    ITySet.fold
      (fun lower_bound (env, acc) ->
        let (env, lower_bound) =
          remove_tyvar_from_lower_bound env var lower_bound
        in
        (env, ITySet.add lower_bound acc))
      lower_bounds
      (env, ITySet.empty)
  in
  let is_not_nothing ty = not @@ TUtils.is_nothing_i env ty in
  let lower_bounds = ITySet.filter is_not_nothing lower_bounds in
  (env, lower_bounds)

(** If a type variable appear in one of its own upper bounds under a combination
    of unions and intersections, it can be simplified away from this upper bound by
    replacing any of its occurences with mixed.
    E.g.
    - if #1 has upper bound (#1 & A), the upper bound can be simplified to
    (mixed & A) = A.
    - if #1 has upper bound (#1 | A), the upper bound can be simplified to
    (mixed | A) = mixed
    *)
let remove_tyvar_from_upper_bound env var upper_bound =
  let is_mixed ty =
    ty_equal ty (MakeType.mixed Reason.none)
    || ty_equal ty (MakeType.intersection Reason.none [])
  in
  let rec remove env ty =
    let (env, ty) = Env.expand_type env ty in
    match deref ty with
    | (_, Tvar v) when Tvid.equal v var -> (env, MakeType.mixed Reason.none)
    | (r, Toption ty) ->
      let (env, ty) = remove env ty in
      (env, MakeType.nullable r ty)
    | (r, Tunion tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let ty =
        if List.exists tyl ~f:is_mixed then
          MakeType.mixed r
        else
          MakeType.union r tyl
      in
      (env, ty)
    | (r, Tnewtype (name, [tyarg], _))
      when String.equal name Naming_special_names.Classes.cSupportDyn ->
      let (env, ty) = remove env tyarg in
      (env, MakeType.supportdyn r ty)
    | (r, Tintersection tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let tyl = List.filter tyl ~f:(fun ty -> not (is_mixed ty)) in
      (env, MakeType.intersection r tyl)
    | _ -> (env, ty)
  and remove_i env ty =
    match ty with
    | LoclType ty ->
      let (env, ty) = remove env ty in
      (env, LoclType ty)
    | _ -> (env, ty)
  in
  remove_i env upper_bound

let remove_tyvar_from_upper_bounds env var upper_bounds =
  let (env, upper_bounds) =
    ITySet.fold
      (fun upper_bound (env, acc) ->
        let (env, upper_bound) =
          remove_tyvar_from_upper_bound env var upper_bound
        in
        (env, ITySet.add upper_bound acc))
      upper_bounds
      (env, ITySet.empty)
  in
  let is_not_mixed ty = not @@ TUtils.is_mixed_i env ty in
  let upper_bounds = ITySet.filter is_not_mixed upper_bounds in
  (env, upper_bounds)

(** Remove a type variable from its upper and lower bounds. More precisely,
    if a type variable appears in one of its bounds under any combination of unions
    and intersections, it can be simplified away from the bound.
    For example,
    - if #1 has lower bound (#1 | A), the lower bound can be simplified to A
    - if #1 has upper bound (#1 | B), the upper bound can be simplified to mixed
    and dually for intersections.
    *)
let remove_tyvar_from_bounds env var =
  Env.log_env_change "remove_tyvar_from_bounds" ~level:3 env
  @@
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let (env, lower_bounds) =
    remove_tyvar_from_lower_bounds env var lower_bounds
  in
  let (env, upper_bounds) =
    remove_tyvar_from_upper_bounds env var upper_bounds
  in
  let env = Env.set_tyvar_lower_bounds env var lower_bounds in
  let env = Env.set_tyvar_upper_bounds env var upper_bounds in
  env

let var_occurs_in_ty env var ty =
  let finder =
    object (this)
      inherit [env * bool] Type_visitor.locl_type_visitor as super

      method! on_tvar (env, occurs) r v =
        let (env, ety) = Env.expand_var env r v in
        match get_node ety with
        | Tvar v -> (env, Tvid.equal v var)
        | _ -> this#on_type (env, occurs) ety

      method! on_type (env, occurs) ty =
        if occurs then
          (env, occurs)
        else
          super#on_type (env, occurs) ty
    end
  in
  finder#on_type (env, false) ty

let err_if_var_in_ty_pure env var ty =
  let (env, var_occurs_in_ty) = var_occurs_in_ty env var ty in
  if var_occurs_in_ty then
    let ty_err =
      Typing_error.(
        primary
        @@ Primary.Unification_cycle
             {
               pos = Env.get_tyvar_pos env var;
               ty_name =
                 lazy
                   Typing_print.(
                     with_blank_tyvars (fun () -> full_rec env var ty));
             })
    in
    (MakeType.union (get_reason ty) [], Some ty_err)
  else
    (ty, None)

let err_if_var_in_ty env var ty =
  match err_if_var_in_ty_pure env var ty with
  | (ty, Some err) ->
    Typing_error_utils.add_typing_error ~env err;
    ty
  | (ty, _) -> ty
