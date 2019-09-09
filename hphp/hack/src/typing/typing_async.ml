(*
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
module Type = Typing_ops
module Env = Typing_env
module TUtils = Typing_utils
module MakeType = Typing_make_type

(* If an expression e is of type `opt_ty_maybe`, then this function
returns the type of `await e`.

There is the special case that
  e : ?Awaitable<T> |- await e : ?T
*)
let overload_extract_from_awaitable env ~p opt_ty_maybe =
  let r = Reason.Rwitness p in
  let rec extract_inner env opt_ty_maybe =
    let (env, e_opt_ty) =
      Typing_solver.expand_type_and_solve
        ~description_of_expected:"an Awaitable"
        env
        p
        opt_ty_maybe
        Errors.unify_error
    in
    match e_opt_ty with
    | (_, Tunion tyl) ->
      (* If we cannot fold the union into a single type, we need to look at
       * all the types *)
      let (env, rtyl) =
        List.fold_right
          ~f:
            begin
              fun ty (env, rtyl) ->
              let (env, rty) = extract_inner env ty in
              (* We have the invariant we'll never have Tunion[Tunion], but
               * the recursive call above can remove a layer of Awaitable, so we need
               * to flatten any Tunion that may have been inside. *)
              let (env, rtyl) = TUtils.flatten_unresolved env rty rtyl in
              (env, rtyl)
            end
          tyl
          ~init:(env, [])
      in
      (env, (r, Tunion rtyl))
    | (_, Toption ty) ->
      (* We want to try to avoid easy double nullables here, so we handle Toption
       * with some special logic. *)
      let (env, ty) = extract_inner env ty in
      TUtils.union env (MakeType.null r) ty
    | (_, Tintersection tyl) ->
      let (env, rtyl) = List.fold_map ~init:env tyl ~f:extract_inner in
      (env, (r, Tintersection rtyl))
    | (r, Tprim Aast.Tnull) -> (env, (r, Tprim Aast.Tnull))
    | (_, Tdynamic) ->
      (* Awaiting a dynamic results in a new dynamic *)
      (env, (r, Tdynamic))
    | ( _,
        ( Terr | Tany _ | Tarraykind _ | Tnonnull | Tprim _ | Tvar _ | Tfun _
        | Tabstract _ | Tclass _ | Ttuple _
        | Tanon (_, _)
        | Tobject | Tshape _ | Tdestructure _ | Tpu _ | Tpu_access _ ) ) ->
      let (env, type_var) = Env.fresh_type env p in
      let expected_type = MakeType.awaitable r type_var in
      let return_type =
        match e_opt_ty with
        | (_, Tany _) -> (r, Typing_defs.make_tany ())
        | (_, Terr) -> (r, Terr)
        | (_, Tdynamic) -> (r, Tdynamic)
        | ( _,
            ( Tnonnull | Tarraykind _ | Tprim _ | Tvar _ | Tfun _ | Tabstract _
            | Tclass _ | Ttuple _ | Tanon _ | Tintersection _ | Toption _
            | Tunion _ | Tobject | Tshape _ | Tdestructure _ | Tpu _
            | Tpu_access _ ) ) ->
          type_var
      in
      let env =
        Type.sub_type
          p
          Reason.URawait
          env
          opt_ty_maybe
          expected_type
          Errors.unify_error
      in
      (env, return_type)
  in
  let env = Env.open_tyvars env p in
  let (env, ty) = extract_inner env opt_ty_maybe in
  let env = Typing_solver.close_tyvars_and_solve env Errors.unify_error in
  (env, ty)

let overload_extract_from_awaitable_list env p tyl =
  List.fold_right
    ~f:
      begin
        fun ty (env, rtyl) ->
        let (env, rty) = overload_extract_from_awaitable env p ty in
        (env, rty :: rtyl)
      end
    tyl
    ~init:(env, [])

let overload_extract_from_awaitable_shape env p fdm =
  Nast.ShapeMap.map_env
    begin
      fun env _key (tk, tv) ->
      let (env, rtv) = overload_extract_from_awaitable env p tv in
      (env, (tk, rtv))
    end
    env
    fdm

let genva env p tyl =
  let (env, rtyl) = overload_extract_from_awaitable_list env p tyl in
  let inner_type = (Reason.Rwitness p, Ttuple rtyl) in
  (env, inner_type)
