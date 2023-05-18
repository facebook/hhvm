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
let overload_extract_from_awaitable_with_ty_err env ~p opt_ty_maybe =
  let r = Reason.Rwitness p in
  let rec extract_inner env opt_ty_maybe =
    let ((env, e1), e_opt_ty) =
      Typing_solver.expand_type_and_solve
        ~description_of_expected:"an Awaitable"
        env
        p
        opt_ty_maybe
    in
    match get_node e_opt_ty with
    | Tunion tyl ->
      let ((env, ty_errs), tyl) =
        List.fold_map
          ~init:(env, Option.to_list e1)
          ~f:(fun (env, ty_errs) ty ->
            match extract_inner env ty with
            | ((env, Some ty_err), ty) -> ((env, ty_err :: ty_errs), ty)
            | ((env, _), ty) -> ((env, ty_errs), ty))
          tyl
      in
      let (env, ty) = TUtils.union_list env r tyl in
      ((env, Typing_error.multiple_opt ty_errs), ty)
    | Toption ty ->
      (* We want to try to avoid easy double nullables here, so we handle Toption
       * with some special logic. *)
      let ((env, e2), ty) = extract_inner env ty in
      let (env, ty) = TUtils.union env (MakeType.null r) ty in
      ((env, Option.merge e1 e2 ~f:Typing_error.both), ty)
    | Tintersection tyl ->
      let ((env, e2), rtyl) =
        TUtils.run_on_intersection_with_ty_err env tyl ~f:extract_inner
      in
      ( (env, Option.merge e1 e2 ~f:Typing_error.both),
        MakeType.intersection r rtyl )
    | Tprim Aast.Tnull -> ((env, e1), e_opt_ty)
    | Tdynamic ->
      (* Awaiting a dynamic results in a new dynamic *)
      ((env, e1), MakeType.dynamic r)
    | Tunapplied_alias _ ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | Tany _
    | Tvec_or_dict _
    | Tnonnull
    | Tprim _
    | Tvar _
    | Tfun _
    | Tgeneric _
    | Tnewtype _
    | Tdependent _
    | Tclass _
    | Ttuple _
    | Tshape _
    | Taccess _
    | Tneg _ ->
      let (env, type_var) =
        if TUtils.is_tyvar_error env e_opt_ty then
          Env.fresh_type_error env p
        else
          Env.fresh_type env p
      in
      let expected_type = MakeType.awaitable r type_var in
      let return_type =
        match get_node e_opt_ty with
        | Tany _ -> mk (r, Typing_defs.make_tany ())
        | Tdynamic -> MakeType.dynamic r
        | Tunapplied_alias _ ->
          Typing_defs.error_Tunapplied_alias_in_illegal_context ()
        | Tnonnull
        | Tvec_or_dict _
        | Tprim _
        | Tvar _
        | Tfun _
        | Tgeneric _
        | Tnewtype _
        | Tdependent _
        | Tclass _
        | Ttuple _
        | Tintersection _
        | Toption _
        | Tunion _
        | Tshape _
        | Taccess _
        | Tneg _ ->
          type_var
      in
      let (env, e2) =
        Type.sub_type
          p
          Reason.URawait
          env
          opt_ty_maybe
          expected_type
          Typing_error.Callback.unify_error
      in
      ((env, Option.merge e1 e2 ~f:Typing_error.both), return_type)
  in
  let env = Env.open_tyvars env p in
  let ((env, ty_err_opt), ty) = extract_inner env opt_ty_maybe in
  let (env, err) = Typing_solver.close_tyvars_and_solve env in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  ((env, err), ty)

let overload_extract_from_awaitable env ~p opt_ty_maybe =
  let ((env, ty_err_opt), res) =
    overload_extract_from_awaitable_with_ty_err env ~p opt_ty_maybe
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, res)

let overload_extract_from_awaitable_list_with_ty_err env p tyl =
  let (env, ty_errs, rtyl) =
    List.fold_right
      ~f:(fun ty (env, ty_errs, rtyl) ->
        let ((env, ty_err_opt), rty) =
          overload_extract_from_awaitable_with_ty_err env ~p ty
        in
        let ty_errs =
          Option.value_map
            ~default:ty_errs
            ~f:(fun e -> e :: ty_errs)
            ty_err_opt
        in
        (env, ty_errs, rty :: rtyl))
      tyl
      ~init:(env, [], [])
  in
  ((env, Typing_error.multiple_opt ty_errs), rtyl)

let overload_extract_from_awaitable_list env p tyl =
  let ((env, ty_err_opt), res) =
    overload_extract_from_awaitable_list_with_ty_err env p tyl
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, res)

let overload_extract_from_awaitable_shape_with_ty_err env p fdm =
  TShapeMap.map_env_ty_err_opt
    (fun env _key (tk, tv) ->
      let (env, rtv) = overload_extract_from_awaitable_with_ty_err env ~p tv in
      (env, (tk, rtv)))
    ~combine_ty_errs:Typing_error.multiple_opt
    env
    fdm

let overload_extract_from_awaitable_shape env p fdm =
  let ((env, ty_err_opt), res) =
    overload_extract_from_awaitable_shape_with_ty_err env p fdm
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, res)
