(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

type t =
  | Primary of primary
  | Apply of callback * t
  | Apply_reasons of reasons_callback * secondary
  | Or of t * t
  | Invalid of Validation_err.t list * t

and primary = Any_prim

and secondary =
  | Of_error of t
  | Violated_constraint of Patt_name.t * Patt_locl_ty.t * Patt_locl_ty.t
  | Subtyping_error of Patt_locl_ty.t * Patt_locl_ty.t
  | Any_snd

and callback = Any_callback

and reasons_callback = Any_reasons_callback [@@deriving show, yojson]

let mark_invalid t errs =
  match (t, errs) with
  | (_, []) -> t
  | (Validated.Valid t, _)
  | (Validated.Invalid t, _) ->
    Validated.Invalid (Invalid (errs, t))

let rec validate ?(env = Validation_env.empty) t =
  match t with
  | Primary prim ->
    let (prim_res, env) = validate_primary prim ~env in
    (Validated.map ~f:(fun prim -> Primary prim) prim_res, env)
  | Apply (cb, t) ->
    let (cb, env) = validate_callback cb ~env in
    let (t, env) = validate t ~env in
    (Validated.map2 ~f:(fun cb t -> Apply (cb, t)) cb t, env)
  | Apply_reasons (rcb, snd) ->
    let (rcb, env) = validate_reasons_callback rcb ~env in
    let (snd, env) = validate_secondary snd ~env in
    (Validated.map2 ~f:(fun rcb snd -> Apply_reasons (rcb, snd)) rcb snd, env)
  | Or (fst, snd) ->
    let (fst, env_fst) = validate fst ~env
    and (snd, env_snd) = validate snd ~env in
    let (env, (errs_fst, errs_snd)) = Validation_env.combine env_fst env_snd in
    let patt_fst_res = mark_invalid fst errs_fst
    and patt_snd_res = mark_invalid snd errs_snd in
    ( Validated.map2
        ~f:(fun patt_fst patt_snd -> Or (patt_fst, patt_snd))
        patt_fst_res
        patt_snd_res,
      env )
  | Invalid _ -> (Validated.invalid t, env)

and validate_primary prim ~env =
  match prim with
  | Any_prim -> (Validated.valid prim, env)

and validate_callback cb ~env =
  match cb with
  | Any_callback -> (Validated.valid cb, env)

and validate_reasons_callback rcb ~env =
  match rcb with
  | Any_reasons_callback -> (Validated.valid rcb, env)

and validate_secondary snd ~env =
  match snd with
  | Any_snd -> (Validated.valid snd, env)
  | Of_error err ->
    let (err, env) = validate err ~env in
    (Validated.map ~f:(fun err -> Of_error err) err, env)
  | Violated_constraint (name, ty_sub, ty_sup) ->
    let (name, env) = Patt_name.validate name ~env in
    let (ty_sub, env) = Patt_locl_ty.validate ty_sub ~env in
    let (ty_sup, env) = Patt_locl_ty.validate ty_sup ~env in
    ( Validated.map3
        ~f:(fun name ty_sub ty_sup ->
          Violated_constraint (name, ty_sub, ty_sup))
        name
        ty_sub
        ty_sup,
      env )
  | Subtyping_error (ty_sub, ty_sup) ->
    let (ty_sub, env) = Patt_locl_ty.validate ty_sub ~env in
    let (ty_sup, env) = Patt_locl_ty.validate ty_sup ~env in
    ( Validated.map2
        ~f:(fun ty_sub ty_sup -> Subtyping_error (ty_sub, ty_sup))
        ty_sub
        ty_sup,
      env )
