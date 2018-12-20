(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Typing_defs

module Reason = Typing_reason
module Type   = Typing_ops
module SubType = Typing_subtype
module Env    = Typing_env
module TUtils = Typing_utils
module SN     = Naming_special_names
module MakeType    = Typing_make_type

(* If an expression e is of type `opt_ty_maybe`, then this function
returns the type of `await e`.

There is the special case that
  e : ?Awaitable<T> |- await e : ?T
*)
let rec overload_extract_from_awaitable ((env,tyvars) as acc) p opt_ty_maybe =
  let r = Reason.Rwitness p in
  let env, e_opt_ty = Env.expand_type env opt_ty_maybe in
  (match e_opt_ty with
  | _, Tunresolved tyl ->
    (* If we cannot fold the union into a single type, we need to look at
     * all the types *)
    let acc, rtyl = List.fold_right ~f:begin fun ty (acc, rtyl) ->
      let (env, tyvars), rty = overload_extract_from_awaitable acc p ty in
      (* We have the invariant we'll never have Tunresolved[Tunresolved], but
       * the recursive call above can remove a layer of Awaitable, so we need
       * to flatten any Tunresolved that may have been inside. *)
      let env, rtyl = TUtils.flatten_unresolved env rty rtyl in
      (env, tyvars), rtyl
    end tyl ~init:(acc, []) in
    acc, (r, Tunresolved rtyl)
  | _, Toption ty ->
    (* We want to try to avoid easy double nullables here, so we handle Toption
     * with some special logic. *)
    let (env, tyvars), ty = overload_extract_from_awaitable (env, tyvars) p ty in
    let env, ty = TUtils.non_null env ty in
    (env, tyvars), (r, Toption ty)
  | r, Tprim Nast.Tnull ->
    acc, (r, Tprim Nast.Tnull)
  | _, Tdynamic -> (* Awaiting a dynamic results in a new dynamic *)
    acc, (r, Tdynamic)
  | _, (Terr | Tany | Tarraykind _ | Tnonnull | Tprim _
    | Tvar _ | Tfun _ | Tabstract _ | Tclass _ | Ttuple _
    | Tanon (_, _) | Tobject | Tshape _ ) ->
    let type_var, tyvars = Env.fresh_type_add_tyvars p tyvars in
    let expected_type = MakeType.awaitable r type_var in
    let return_type = match e_opt_ty with
      | _, Tany -> r, Tany
      | _, Terr -> r, Terr
      | _, Tdynamic -> r, Tdynamic
      | _, (Tnonnull | Tarraykind _ | Tprim _ | Tvar _ | Tfun _
        | Tabstract _ | Tclass _ | Ttuple _ | Tanon _
        | Toption _ | Tunresolved _ | Tobject | Tshape _) -> type_var
    in
    let env = Type.sub_type p Reason.URawait env opt_ty_maybe expected_type in
    (env, tyvars), return_type
  )

let overload_extract_from_awaitable_list acc p tyl =
  List.fold_right ~f:begin fun ty (acc, rtyl) ->
    let acc, rty =  overload_extract_from_awaitable acc p ty in
    acc, rty::rtyl
  end tyl ~init:(acc, [])

let overload_extract_from_awaitable_shape acc p fdm =
  Nast.ShapeMap.map_env begin fun acc _key (tk, tv) ->
      let acc, rtv = overload_extract_from_awaitable acc p tv in
      acc, (tk, rtv)
  end acc fdm

let gena (env, tyvars) p ty =
  let env, ty = TUtils.fold_unresolved env ty in
  let acc = (env, tyvars) in
  match ty with
  | _, Tarraykind (AKany | AKempty) ->
    acc, ty
  | r, Tarraykind (AKvec ty1) ->
    let acc, ty1 = overload_extract_from_awaitable acc p ty1 in
    acc, (r, Tarraykind (AKvec ty1))
  | r, Tarraykind (AKvarray ty1) ->
    let acc, ty1 = overload_extract_from_awaitable acc p ty1 in
    acc, (r, Tarraykind (AKvarray ty1))
  | r, Tarraykind (AKvarray_or_darray ty1) ->
    let acc, ty1 = overload_extract_from_awaitable acc p ty1 in
    acc, (r, Tarraykind (AKvarray_or_darray ty1))
  | r, Tarraykind AKmap (ty1, ty2) ->
    let acc, ty2 = overload_extract_from_awaitable acc p ty2 in
    acc, (r, Tarraykind (AKmap (ty1, ty2)))
  | r, Tarraykind AKdarray (ty1, ty2) ->
    let acc, ty2 = overload_extract_from_awaitable acc p ty2 in
    acc, (r, Tarraykind (AKdarray (ty1, ty2)))
  | r, Tarraykind AKshape fdm ->
    let acc, fdm = overload_extract_from_awaitable_shape acc p fdm in
    acc, (r, Tarraykind (AKshape fdm))
  | r, Ttuple tyl ->
    let acc, tyl =
      overload_extract_from_awaitable_list acc p tyl in
    acc, (r, Ttuple tyl)
  | r, ty ->
    (* Oh well...let's at least make sure it is array-ish *)
    let expected_ty = r, Tarraykind AKany in
    let env =
      Errors.try_
        (fun () -> Type.sub_type p Reason.URawait env (r, ty) expected_ty)
        (fun _ ->
          let ty_str = Typing_print.error ty in
          Errors.gena_expects_array p (Reason.to_pos r) ty_str;
          env
        )
    in
    (env, tyvars), expected_ty

let genva acc p tyl =
  let acc, rtyl =
    overload_extract_from_awaitable_list acc p tyl in
  let inner_type = (Reason.Rwitness p, Ttuple rtyl) in
  acc, inner_type

let rec gen_array_rec ((env,_tyvars) as acc) p ty =
  let rec is_array ((env,tyvars) as acc) ty = begin
    let env, ty = TUtils.fold_unresolved env ty in
    match ty with
      | _, Ttuple _
      | _, Tarraykind _ -> gen_array_rec (env, tyvars) p ty
      | r, Tunresolved tyl -> begin
        (* You can run gen_array_rec on heterogeneous arrays, like this one:
         * array(
         *   'foo' => cached_result(1),
         *   'bar' => array(
         *     'baz' => cached_result(2),
         *   ),
         * )
         *
         * In this case the value type in the array will be unresolved; we need
         * to check all the types in the unresolved. *)
        let acc, rtyl = List.fold_right ~f:begin fun ty (acc, rtyl) ->
          let acc, ty = is_array acc ty in
          acc, ty::rtyl
        end tyl ~init:(acc, []) in
        acc, (r, Tunresolved rtyl)
      end
      | _, (Terr | Tany | Tnonnull | Tprim _ | Toption _ | Tvar _
        | Tfun _ | Tabstract _ | Tclass _ | Tanon _ | Tobject
        | Tshape _ | Tdynamic
           ) -> overload_extract_from_awaitable acc p ty
  end in
  match snd (TUtils.fold_unresolved env ty) with
  | r, Tarraykind (AKvec vty) ->
    let acc, vty = is_array acc vty in
    acc, (r, Tarraykind (AKvec vty))
  | r, Tarraykind (AKvarray vty) ->
    let acc, vty = is_array acc vty in
    acc, (r, Tarraykind (AKvarray vty))
  | r, Tarraykind (AKvarray_or_darray vty) ->
    let acc, vty = is_array acc vty in
    acc, (r, Tarraykind (AKvarray_or_darray vty))
  | r, Tarraykind (AKmap (kty, vty)) ->
    let acc, vty = is_array acc vty in
    acc, (r, Tarraykind (AKmap( kty, vty)))
  | r, Tarraykind (AKdarray (kty, vty)) ->
    let acc, vty = is_array acc vty in
    acc, (r, Tarraykind (AKdarray(kty, vty)))
  | r, Tarraykind (AKshape fdm) ->
    let acc, fdm = Nast.ShapeMap.map_env begin fun acc _key (tk, tv) ->
      let acc, tv = is_array acc tv in
      acc, (tk, tv)
    end acc fdm in
    acc, (r, Tarraykind (AKshape fdm))
  | _, Ttuple tyl -> gen_array_va_rec acc p tyl
  | _, (Terr | Tany | Tnonnull | Tarraykind _ | Tprim _ | Toption _
    | Tvar _ | Tfun _ | Tabstract _ | Tclass _ | Tdynamic
    | Tanon (_, _) | Tunresolved _ | Tobject | Tshape _
       ) -> gena acc p ty

and gen_array_va_rec acc p tyl =
  (* For each item in the type list, treat it differently *)
  let rec gen_array_va_rec' ((env, _tyvars) as acc) ty =
  (* Unwrap option types (hopefully we won't have option options *)
    (match snd (TUtils.fold_unresolved env ty) with
    | r, Toption opt_ty ->
      let (env, tyvars), opt_ty = gen_array_va_rec' acc opt_ty in
      let env, opt_ty = TUtils.non_null env opt_ty in
      (env, tyvars), (r, Toption opt_ty)
    | _, Tarraykind _ -> gen_array_rec acc p ty
    | _, Ttuple tyl -> genva acc p tyl
    | _, (Terr | Tany | Tnonnull | Tprim _ | Tvar _ | Tfun _ | Tdynamic
      | Tabstract _ | Tclass _ | Tanon _ |  Tunresolved _
      | Tobject | Tshape _) ->
       overload_extract_from_awaitable acc p ty) in

  let acc, rtyl = List.fold_right ~f:begin fun ty (acc, rtyl) ->
    let acc, ty = gen_array_va_rec' acc ty in
    acc, ty::rtyl
  end tyl ~init:(acc, []) in
  acc, (Reason.Rwitness p, Ttuple rtyl)
