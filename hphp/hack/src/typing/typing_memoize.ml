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
module MakeType = Typing_make_type

let check_param : env -> Nast.fun_param -> unit =
 fun env { param_type_hint; param_pos; param_name; _ } ->
  let error ty =
    let ty_name = lazy (Typing_print.error env ty) in
    let msgl =
      Lazy.map ty_name ~f:(fun ty_name ->
          Reason.to_string ("This is " ^ ty_name) (get_reason ty))
    in
    Typing_error.Primary.Invalid_memoized_param
      { pos = param_pos; reason = msgl; ty_name }
  in
  let check_memoizable env pos ty =
    let rec check_memoizable : env -> locl_ty -> unit =
     fun env ty ->
      let (env, ty) = Env.expand_type env ty in
      let ety_env = empty_expand_env in
      let ty = Typing_utils.strip_dynamic env ty in
      let ((env, ty_err_opt), ty, _) =
        Typing_tdef.force_expand_typedef ~ety_env env ty
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      if not (Typing_utils.is_tyvar_error env ty) then
        match get_node ty with
        | Tprim (Tnull | Tarraykey | Tbool | Tint | Tfloat | Tstring | Tnum) ->
          ()
        | Tnonnull
        | Tany _
        | Tdynamic
        | Tneg _ ->
          let (_ : env) = Typing_local_ops.enforce_memoize_object pos env in
          ()
        | Tprim (Tvoid | Tresource | Tnoreturn) ->
          Typing_error_utils.add_typing_error ~env
          @@ Typing_error.primary
          @@ error ty
        | Toption ty -> check_memoizable env ty
        | Ttuple tyl -> List.iter tyl ~f:(check_memoizable env)
        (* Just accept all generic types for now. Stricter check_memoizables to come later. *)
        | Tgeneric _ ->
          (* FIXME fun fact:
             the comment above about "stricter check_memoizables to come later" was added in revision
             in August 2015 *)
          ()
        | Tnewtype (id, _, _) when String.equal SN.Classes.cEnumClassLabel id ->
          (* EnumClassLabels are memoizable *)
          ()
        | Tnewtype (name, [ty], _) when String.equal name SN.Classes.cSupportDyn
          ->
          check_memoizable env ty
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
          List.iter tyl ~f:(check_memoizable env)
        | Tvec_or_dict (_, ty) -> check_memoizable env ty
        | Tshape { s_fields = fdm; _ } ->
          (* TODO(shapes) should unknown type affect memoizability? *)
          TShapeMap.iter
            begin
              (fun _ { sft_ty; _ } -> check_memoizable env sft_ty)
            end
            fdm
        | Tclass _ ->
          let env = Env.open_tyvars env pos in
          let (env, type_param) = Env.fresh_type env pos in
          let container_type = MakeType.container Reason.none type_param in
          let (env, ty_err_opt) =
            Typing_utils.sub_type env ty container_type
            @@ Some (Typing_error.Reasons_callback.unify_error_at pos)
          in
          let is_container = Option.is_none ty_err_opt in

          let mixed = MakeType.mixed Reason.none in
          let hackarray = MakeType.any_array Reason.none mixed mixed in
          let is_hackarray = Typing_utils.is_sub_type env ty hackarray in
          let env =
            if not is_hackarray then
              (* Check if it's a UNSAFEsingleton memoize param *)
              let singleton =
                MakeType.class_type
                  Reason.none
                  SN.Classes.cUNSAFESingletonMemoizeParam
                  []
              in
              if Typing_utils.is_sub_type env ty singleton then
                env
              else
                Typing_local_ops.enforce_memoize_object pos env
            else
              env
          in
          let env = Env.set_tyvar_variance env container_type in
          let (env, e1) = Typing_solver.close_tyvars_and_solve env in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e1;
          if is_container then
            check_memoizable env type_param
          else
            let base_error = error ty in
            let ((env, ty_err_opt), (tfty, _tal)) =
              Typing_object_get.obj_get
                ~obj_pos:param_pos
                ~is_method:true
                ~inst_meth:false
                ~meth_caller:false
                ~nullsafe:None
                ~coerce_from_ty:None
                ~explicit_targs:[]
                ~class_id:
                  (CIexpr
                     ( (),
                       param_pos,
                       Lvar (param_pos, Local_id.make_unscoped param_name) ))
                ~member_id:(pos, SN.Members.mGetInstanceKey)
                ~on_error:Typing_error.Callback.(always base_error)
                env
                ty
            in
            Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
            ignore
              (Typing.call
                 ~expected:None
                 ~expr_pos:pos
                 ~recv_pos:pos
                 env
                 tfty
                 []
                 None)
        | Tunapplied_alias _ ->
          Typing_defs.error_Tunapplied_alias_in_illegal_context ()
        | Taccess _ -> ()
        | Tfun _
        | Tvar _ ->
          Typing_error_utils.add_typing_error ~env
          @@ Typing_error.primary
          @@ error ty
    in
    check_memoizable env ty
  in
  match hint_of_type_hint param_type_hint with
  | None -> ()
  | Some hint ->
    let (hint_pos, _) = hint in
    let ((env, ty_err_opt), ty) =
      Typing_phase.localize_hint_no_subst env ~ignore_errors:true hint
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    check_memoizable env hint_pos ty

let check : env -> Nast.user_attribute list -> Nast.fun_param list -> unit =
 fun env user_attributes params ->
  if
    Naming_attributes.mem SN.UserAttributes.uaMemoize user_attributes
    || Naming_attributes.mem SN.UserAttributes.uaMemoizeLSB user_attributes
  then
    List.iter ~f:(check_param env) params

let check_function : env -> Nast.fun_ -> unit =
 fun env { f_user_attributes; f_params; _ } ->
  check env f_user_attributes f_params

let check_method : env -> Nast.method_ -> unit =
 fun env { m_user_attributes; m_params; _ } ->
  check env m_user_attributes m_params
