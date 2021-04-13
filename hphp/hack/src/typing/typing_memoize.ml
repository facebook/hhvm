(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Aast
open Typing_defs
open Typing_env_types
module Env = Typing_env
module SN = Naming_special_names
module SubType = Typing_subtype
module MakeType = Typing_make_type

let check_param : env -> Nast.fun_param -> unit =
 fun env { param_type_hint; param_pos; param_name; _ } ->
  let error ty =
    let ty_str = Typing_print.error env ty in
    let msgl = Reason.to_string ("This is " ^ ty_str) (get_reason ty) in
    Errors.invalid_memoized_param param_pos msgl
  in
  let check_memoizable env pos ty =
    let rec check_memoizable : env -> locl_ty -> unit =
     fun env ty ->
      let (env, ty) = Env.expand_type env ty in
      let ety_env =
        Typing_phase.env_with_self env ~on_error:Errors.ignore_error
      in
      let (env, ty, _) = Typing_tdef.force_expand_typedef ~ety_env env ty in
      match get_node ty with
      | Tprim (Tnull | Tarraykey | Tbool | Tint | Tfloat | Tstring | Tnum)
      | Tnonnull
      | Tany _
      | Terr
      | Tdynamic ->
        ()
      | Tprim (Tvoid | Tresource | Tnoreturn) -> error ty
      | Toption ty -> check_memoizable env ty
      | Ttuple tyl -> List.iter tyl (check_memoizable env)
      (* Just accept all generic types for now. Stricter check_memoizables to come later. *)
      | Tgeneric _ ->
        (* FIXME fun fact:
        the comment above about "stricter check_memoizables to come later" was added in revision
        in August 2015 *)
        ()
      (* For parameter type 'this::TID' defined by 'type const TID as Bar' check_memoizables
       * Bar recursively. Also enums represented using AKnewtype.
       *)
      | Tnewtype (_, _, ty)
      | Tdependent (_, ty) ->
        check_memoizable env ty
      (* Handling Tunion and Tintersection case here for completeness, even though it
       * shouldn't be possible to have an unresolved type when check_memoizableing
       * the method declaration. No corresponding test case for this.
       *)
      | Tunion tyl
      | Tintersection tyl ->
        List.iter tyl (check_memoizable env)
      | Tvarray ty
      | Tdarray (_, ty)
      | Tvec_or_dict (_, ty)
      | Tvarray_or_darray (_, ty) ->
        check_memoizable env ty
      | Tshape (_, fdm) ->
        TShapeMap.iter
          begin
            fun _ { sft_ty; _ } ->
            check_memoizable env sft_ty
          end
          fdm
      | Tclass _ ->
        let env = Env.open_tyvars env pos in
        let (env, type_param) = Env.fresh_type env pos in
        let container_type = MakeType.container Reason.none type_param in
        let (env, props) =
          SubType.simplify_subtype_i
            env
            (LoclType ty)
            (LoclType container_type)
            (Errors.unify_error_at pos)
        in
        let (env, prop) =
          SubType.prop_to_env env props (Errors.unify_error_at pos)
        in
        let is_container = Typing_logic.is_valid prop in
        let env = Env.set_tyvar_variance env container_type in
        let env = Typing_solver.close_tyvars_and_solve env in
        if is_container then
          check_memoizable env type_param
        else
          let (env, (tfty, _tal)) =
            Typing_object_get.obj_get
              ~obj_pos:param_pos
              ~is_method:true
              ~inst_meth:false
              ~nullsafe:None
              ~coerce_from_ty:None
              ~explicit_targs:[]
              ~class_id:
                (CIexpr
                   ( param_pos,
                     Lvar (param_pos, Local_id.make_unscoped param_name) ))
              ~member_id:(pos, SN.Members.mGetInstanceKey)
              ~on_error:(fun ?code:_ _ _ -> error ty)
              env
              ty
          in
          ignore (Typing.call ~expected:None pos env tfty [] None)
      | Tunapplied_alias _ ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | Taccess _ -> ()
      | Tfun _
      | Tvar _
      | Tobject ->
        error ty
    in
    check_memoizable env ty
  in
  match hint_of_type_hint param_type_hint with
  | None -> ()
  | Some hint ->
    let (hint_pos, _) = hint in
    let (env, ty) =
      Typing_phase.localize_hint_with_self env ~ignore_errors:true hint
    in
    check_memoizable env hint_pos ty

let check :
    env ->
    Nast.user_attribute list ->
    Nast.fun_param list ->
    Nast.fun_variadicity ->
    unit =
 fun env user_attributes params variadic ->
  if
    Naming_attributes.mem SN.UserAttributes.uaMemoize user_attributes
    || Naming_attributes.mem SN.UserAttributes.uaMemoizeLSB user_attributes
  then (
    List.iter ~f:(check_param env) params;
    match variadic with
    | FVvariadicArg vparam -> check_param env vparam
    | FVellipsis _
    | FVnonVariadic ->
      ()
  )

let check_function : env -> Nast.fun_ -> unit =
 fun env { f_user_attributes; f_params; f_variadic; _ } ->
  check env f_user_attributes f_params f_variadic

let check_method : env -> Nast.method_ -> unit =
 fun env { m_user_attributes; m_params; m_variadic; _ } ->
  check env m_user_attributes m_params m_variadic
