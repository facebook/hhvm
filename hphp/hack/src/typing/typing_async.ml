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
let overload_extract_from_awaitable env ~p opt_ty_maybe =
  let r = Reason.Rwitness p in
  let rec extract_inner env opt_ty_maybe =
    let (env, e_opt_ty) =
      Typing_solver.expand_type_and_solve
        ~description_of_expected:"an Awaitable"
        env
        p
        opt_ty_maybe
    in
    match get_node e_opt_ty with
    | Tunion tyl ->
      let (env, tyl) = List.fold_map ~init:env ~f:extract_inner tyl in
      TUtils.union_list env r tyl
    | Toption ty ->
      (* We want to try to avoid easy double nullables here, so we handle Toption
       * with some special logic. *)
      let (env, ty) = extract_inner env ty in
      TUtils.union env (MakeType.null r) ty
    | Tintersection tyl ->
      let (env, rtyl) = TUtils.run_on_intersection env tyl ~f:extract_inner in
      (env, MakeType.intersection r rtyl)
    | Tprim Aast.Tnull -> (env, e_opt_ty)
    | Tdynamic ->
      (* Awaiting a dynamic results in a new dynamic *)
      (env, MakeType.dynamic r)
    | Tunapplied_alias _ ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | Terr
    | Tany _
    | Tvarray _
    | Tdarray _
    | Tvarray_or_darray _
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
    | Tobject
    | Tshape _
    | Taccess _
    | Tneg _ ->
      let (env, type_var) = Env.fresh_type env p in
      let expected_type = MakeType.awaitable r type_var in
      let return_type =
        match get_node e_opt_ty with
        | Tany _ -> mk (r, Typing_defs.make_tany ())
        | Terr -> MakeType.err r
        | Tdynamic -> MakeType.dynamic r
        | Tunapplied_alias _ ->
          Typing_defs.error_Tunapplied_alias_in_illegal_context ()
        | Tnonnull
        | Tvarray _
        | Tdarray _
        | Tvarray_or_darray _
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
        | Tobject
        | Tshape _
        | Taccess _
        | Tneg _ ->
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
  let env = Typing_solver.close_tyvars_and_solve env in
  (env, ty)

let overload_extract_from_awaitable_list env p tyl =
  List.fold_right
    ~f:
      begin
        fun ty (env, rtyl) ->
        let (env, rty) = overload_extract_from_awaitable env ~p ty in
        (env, rty :: rtyl)
      end
    tyl
    ~init:(env, [])

let overload_extract_from_awaitable_shape env p fdm =
  TShapeMap.map_env
    begin
      fun env _key (tk, tv) ->
      let (env, rtv) = overload_extract_from_awaitable env ~p tv in
      (env, (tk, rtv))
    end
    env
    fdm
