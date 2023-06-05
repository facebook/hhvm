(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module V = Validated
open Core

type t =
  | Apply of {
      patt_name: Patt_name.t;
      patt_params: params;
    }
  | Prim of prim
  | Shape of shape_fields
  | Option of t
  | Tuple of t list
  | Dynamic
  | Nonnull
  | Any
  | Or of {
      patt_fst: t;
      patt_snd: t;
    }
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
  | Invalid of Validation_err.t list * t

and params =
  | Nil
  | Wildcard
  | Cons of {
      patt_hd: t;
      patt_tl: params;
    }
  | Exists of t

and prim =
  | Null
  | Void
  | Int
  | Bool
  | Float
  | String
  | Resource
  | Num
  | Arraykey
  | Noreturn

and shape_fields =
  | Fld of {
      patt_fld: shape_field;
      patt_rest: shape_fields;
    }
  | Open
  | Closed

and shape_field = {
  lbl: shape_label;
  optional: bool;
  patt: t;
}

and shape_label =
  | StrLbl of string
  | IntLbl of string
  | CConstLbl of {
      cls_nm: string;
      cnst_nm: string;
    }
[@@deriving compare, eq, sexp, show, yojson]

let mark_invalid t errs =
  match (t, errs) with
  | (_, []) -> t
  | (V.Valid t, _)
  | (V.Invalid t, _) ->
    V.Invalid (Invalid (errs, t))

let rec validate ?(env = Validation_env.empty) t =
  match t with
  | As { lbl; patt } ->
    let (env, errs) =
      match Validation_env.add ~key:lbl ~data:Patt_binding_ty.Ty env with
      | Error env -> (env, Validation_err.[Shadowed lbl])
      | Ok env -> (env, [])
    in
    let (patt, env) = validate patt ~env in
    let t = V.map ~f:(fun patt -> As { lbl; patt }) patt in
    (mark_invalid t errs, env)
  | Apply { patt_name; patt_params } ->
    let (patt_name_res, env) = Patt_name.validate patt_name ~env in
    let (patt_params_res, env) = validate_params patt_params ~env in
    ( V.map2
        ~f:(fun patt_name patt_params -> Apply { patt_name; patt_params })
        patt_name_res
        patt_params_res,
      env )
  | Shape shape_fields ->
    let (shape_fields_res, env) = validate_shape_fields shape_fields ~env in
    (V.map ~f:(fun shape_fields -> Shape shape_fields) shape_fields_res, env)
  | Option t ->
    let (t_res, env) = validate t ~env in
    (V.map ~f:(fun t -> Option t) t_res, env)
  | Tuple ts ->
    let (ts_res, env) = validate_tuple ts ~env in
    (V.map ~f:(fun ts -> Tuple ts) ts_res, env)
  | Or { patt_fst; patt_snd } ->
    let (patt_fst_res, env_fst) = validate patt_fst ~env
    and (patt_snd_res, env_snd) = validate patt_snd ~env in
    let (env, (errs_fst, errs_snd)) = Validation_env.combine env_fst env_snd in
    let patt_fst_res = mark_invalid patt_fst_res errs_fst
    and patt_snd_res = mark_invalid patt_snd_res errs_snd in
    ( V.map2
        ~f:(fun patt_fst patt_snd -> Or { patt_fst; patt_snd })
        patt_fst_res
        patt_snd_res,
      env )
  | Invalid _ -> (V.Invalid t, env)
  | Prim _
  | Dynamic
  | Nonnull
  | Any ->
    (V.Valid t, env)

and validate_tuple ts ~env =
  let rec aux ts ~k ~env =
    match ts with
    | [] -> k (V.valid [], env)
    | next :: rest ->
      let (next_res, env) = validate next ~env in
      aux rest ~env ~k:(fun (rest_res, env) ->
          k (V.map2 ~f:(fun next rest -> next :: rest) next_res rest_res, env))
  in
  aux ts ~env ~k:(fun x -> x)

and validate_params params ~env =
  match params with
  | Nil
  | Wildcard ->
    (V.Valid params, env)
  | Exists t ->
    let (t_res, env) = validate t ~env in
    (V.map ~f:(fun t -> Exists t) t_res, env)
  | Cons { patt_hd; patt_tl } ->
    let (patt_hd_res, env) = validate patt_hd ~env in
    let (patt_tl_res, env) = validate_params patt_tl ~env in
    ( V.map2
        ~f:(fun patt_hd patt_tl -> Cons { patt_hd; patt_tl })
        patt_hd_res
        patt_tl_res,
      env )

and validate_shape_fields shape_fields ~env =
  match shape_fields with
  | Fld { patt_fld; patt_rest } ->
    let (patt_fld_res, env) = validate_shape_field patt_fld ~env in
    let (patt_rest_res, env) = validate_shape_fields patt_rest ~env in
    ( V.map2
        ~f:(fun patt_fld patt_rest -> Fld { patt_fld; patt_rest })
        patt_fld_res
        patt_rest_res,
      env )
  | Open
  | Closed ->
    (V.Valid shape_fields, env)

and validate_shape_field shape_field ~env =
  let (patt_res, env) = validate shape_field.patt ~env in
  (V.map ~f:(fun patt -> { shape_field with patt }) patt_res, env)
