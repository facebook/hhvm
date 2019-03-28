(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Nast
open Typing_defs

module Env = Typing_env
module SN = Naming_special_names
module SubType = Typing_subtype
module MakeType = Typing_make_type

let check_param : Env.env -> Nast.fun_param -> unit =
  fun env {param_hint; param_pos; _} ->
  let error ty =
    let ty_str = Typing_print.error env ty in
    let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
    Errors.invalid_memoized_param param_pos msgl
  in
  let rec check_memoizable: Env.env -> locl ty -> unit =
    fun env ty ->
    let env, ty = Env.expand_type env ty in
    match ty with
    | _, (Tprim (Tnull | Tarraykey | Tbool | Tint | Tfloat | Tstring | Tnum)
         | Tnonnull | Tany | Terr | Tabstract (AKenum _, _) | Tdynamic) ->
       ()
    | _, Tprim (Tvoid | Tresource | Tnoreturn) -> error ty
    | _, Toption ty -> check_memoizable env ty
    | _, Ttuple tyl -> List.iter tyl (check_memoizable env)
    | _, Tabstract (AKnewtype (_, _), _) ->
      let env, t', _ =
        let ety_env = Typing_phase.env_with_self env in
        Typing_tdef.force_expand_typedef ~ety_env env ty in
      check_memoizable env t'
    (* Just accept all generic types for now. Stricter check_memoizables to come later. *)
    | _, Tabstract (AKgeneric _, _) ->
      ()
    (* For parameter type 'this::TID' defined by 'type const TID as Bar' check_memoizables
     * Bar recursively.
     *)
    | _, Tabstract (AKdependent _, Some ty) -> check_memoizable env ty
    (* Allow unconstrined dependent type `abstract type const TID` just as we
     * allow unconstrained generics. *)
    | _, Tabstract (AKdependent _, None) -> ()
    (* Handling Tunresolved case here for completeness, even though it
     * shouldn't be possible to have an unresolved type when check_memoizableing
     * the method declaration. No corresponding test case for this.
     *)
    | _, Tunresolved tyl -> List.iter tyl (check_memoizable env)
    (* Allow untyped arrays. *)
    | _, Tarraykind AKany
    | _, Tarraykind AKempty ->
        ()
    | _,
      Tarraykind (
        AKvarray ty
        | AKvec ty
        | AKdarray(_, ty)
        | AKvarray_or_darray ty
        | AKmap(_, ty)
      ) ->
        check_memoizable env ty
    | _, Tshape (_, fdm) ->
      ShapeMap.iter begin fun _ {sft_ty; _} ->
        check_memoizable env sft_ty
      end fdm
    | r, Tclass _ ->
      let p = Reason.to_pos r in
      let env = Env.open_tyvars env p in
      let env, type_param = Env.fresh_unresolved_type env p in
      let container_type = MakeType.container Reason.none type_param in
      let env, is_container =
        Errors.try_
          (fun () ->
            SubType.sub_type env ty container_type, true)
          (fun _ -> env, false) in
      let env = Env.set_tyvar_variance env container_type in
      let env = SubType.close_tyvars_and_solve env in
      if is_container then
        check_memoizable env type_param
      else
        let r, _ = ty in
        let memoizable_type = MakeType.class_type r SN.Classes.cIMemoizeParam [] in
        if SubType.is_sub_type env ty memoizable_type
        then ()
        else error ty;
    | _, Tfun _
    | _, Tvar _
    | _, Tanon (_, _)
    | _, Tobject -> error ty
  in
  match param_hint with
  | None -> ()
  | Some hint ->
    let env, ty = Typing_phase.localize_hint_with_self env hint in
    check_memoizable env ty

let check: Env.env -> Nast.user_attribute list ->
    Nast.fun_param list -> Nast.fun_variadicity -> unit =
  fun env user_attributes params variadic ->
  if Attributes.mem SN.UserAttributes.uaMemoize user_attributes ||
     Attributes.mem SN.UserAttributes.uaMemoizeLSB user_attributes
  then begin
    List.iter ~f:(check_param env) params;
    match variadic with
    | FVvariadicArg vparam -> check_param env vparam
    | FVellipsis _
    | FVnonVariadic -> ()
  end

let check_function: Env.env -> Nast.fun_ -> unit =
  fun env {f_user_attributes; f_params; f_variadic; _ } ->
  check env f_user_attributes f_params f_variadic

let check_method: Env.env -> Nast.method_ -> unit =
  fun env {m_user_attributes; m_params; m_variadic; _ } ->
  check env m_user_attributes m_params m_variadic
