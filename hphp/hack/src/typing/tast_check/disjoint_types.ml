(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Common
open Aast
open Typing_defs
module Cls = Folded_class
module Env = Tast_env
module SN = Naming_special_names

(** Check for the __NonDisjoint attribute on type parameters of functions and methods.
  All type parameters with this attribute should be infered to types which are not disjoint. *)

let warning_kind = Typing_warning.Non_disjoint_check

let error_codes = Typing_warning_utils.codes warning_kind

let add_warning env p name ty1 ty2 ~dynamic ~as_lint =
  let as_lint =
    if as_lint then
      Some None
    else
      None
  in
  Typing_warning_utils.add_for_migration
    (Env.get_tcopt env)
    ~as_lint
    ( p,
      warning_kind,
      {
        Typing_warning.NonDisjointCheck.name = Utils.strip_ns name;
        dynamic;
        ty1 = Env.print_ty env ty1;
        ty2 = Env.print_ty env ty2;
      } )

let check_non_disjoint ~is_dynamic_call ~as_lint env p name ty1 ty2 =
  let tenv = Tast_env.tast_env_as_typing_env env in
  if
    (not is_dynamic_call)
    && Typing_utils.(
         (not (is_nothing tenv ty1))
         && (not (is_nothing tenv ty2))
         && is_type_disjoint tenv ty1 ty2)
  then
    add_warning env p name ty1 ty2 ~dynamic:false ~as_lint
  else
    let ty1 = Tast_env.strip_dynamic env ty1 in
    let ty2 = Tast_env.strip_dynamic env ty2 in
    if
      Typing_utils.(
        (not (is_nothing tenv ty1))
        && (not (is_nothing tenv ty2))
        && is_type_disjoint tenv ty1 ty2)
    then
      add_warning env p name ty1 ty2 ~dynamic:true ~as_lint

let rec check_non_disjoint_tyl ~is_dynamic_call ~as_lint env p name tyl =
  match tyl with
  | [] -> ()
  | ty :: tyl ->
    List.iter
      tyl
      ~f:(check_non_disjoint ~is_dynamic_call ~as_lint env p name ty);
    check_non_disjoint_tyl ~is_dynamic_call ~as_lint env p name tyl

let has_non_disjoint_attr tp =
  Attributes.mem SN.UserAttributes.uaNonDisjoint tp.tp_user_attributes

let check_function_type_args_non_disjoint
    ~is_dynamic_call ~as_lint env p name tal fun_ty =
  match get_node fun_ty with
  | Tfun { ft_tparams = tpl; _ } ->
    if List.exists tpl ~f:has_non_disjoint_attr then
      let (pairs, _) = List.zip_with_remainder tpl tal in
      let tyl =
        List.filter_map pairs ~f:(fun (tp, (ty, _)) ->
            if has_non_disjoint_attr tp then
              Some ty
            else
              None)
      in
      check_non_disjoint_tyl ~is_dynamic_call ~as_lint env p name tyl
  | _ -> ()

let member_hook ~is_dynamic_call ~as_lint env p class_type method_name tal =
  match Cls.get_method class_type method_name with
  | None -> ()
  | Some { ce_type = (lazy fun_ty); _ } ->
    check_function_type_args_non_disjoint
      ~is_dynamic_call
      ~as_lint
      env
      p
      method_name
      tal
      fun_ty

let check_instance_method_call
    env p ~as_lint function_type receiver_type method_name tal =
  let ctx = Tast_env.get_ctx env in
  let tenv = Env.tast_env_as_typing_env env in
  let is_dynamic_call =
    Option.is_some (Typing_utils.try_strip_dynamic tenv function_type)
  in
  Tast_env.get_class_ids env receiver_type
  |> List.filter_map ~f:(fun c ->
         Decl_provider.get_class ctx c |> Decl_entry.to_option)
  |> List.iter ~f:(fun class_type ->
         member_hook ~is_dynamic_call ~as_lint env p class_type method_name tal)

let check_static_method_call env p ~as_lint class_result method_name tal =
  match class_result with
  | Decl_entry.Found cd -> begin
    match Cls.get_smethod cd method_name with
    | None -> ()
    | Some { ce_type = (lazy fun_ty); _ } ->
      check_function_type_args_non_disjoint
        ~is_dynamic_call:false
        ~as_lint
        env
        p
        method_name
        tal
        fun_ty
  end
  | _ -> ()

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      (* Function call *)
      | (_, p, Call { func = (_, _, Id (_, name)); targs = _ :: _ as tal; _ })
        -> begin
        match Decl_provider.get_fun (Tast_env.get_ctx env) name with
        | Decl_entry.Found { fe_type; _ } ->
          check_function_type_args_non_disjoint
            ~is_dynamic_call:false
            ~as_lint
            env
            p
            name
            tal
            fe_type
        | _ -> ()
      end
      (* Method call *)
      | ( _,
          p,
          Call
            {
              func =
                ( function_type,
                  _,
                  Obj_get
                    ( (receiver_type, _, _),
                      (_, _, Id (_, method_name)),
                      _,
                      Aast.Is_method ) );
              targs = _ :: _ as tal;
              _;
            } ) ->
        check_instance_method_call
          env
          p
          ~as_lint
          function_type
          receiver_type
          method_name
          tal
      (* Static method call *)
      | ( _,
          p,
          Call
            {
              func = (_, _, Class_const ((_, _, ci), (_, method_name)));
              targs = _ :: _ as tal;
              _;
            } ) ->
        let tenv = Env.tast_env_as_typing_env env in
        let class_result =
          match ci with
          | CI cls -> Typing_env.get_class tenv (snd cls)
          | CIself
          | CIstatic ->
            Typing_env.get_self_class tenv
          | CIparent -> Typing_env.get_parent_class tenv
          | _ -> Decl_entry.DoesNotExist
        in

        check_static_method_call env p ~as_lint class_result method_name tal
      | _ -> ()
  end
