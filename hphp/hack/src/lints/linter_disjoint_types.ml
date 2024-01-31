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
module Cls = Decl_provider.Class
module Env = Tast_env
module SN = Naming_special_names

let check_non_disjoint env p name ty1 ty2 =
  let tenv = Tast_env.tast_env_as_typing_env env in
  if
    Typing_utils.(
      (not (is_nothing tenv ty1))
      && (not (is_nothing tenv ty2))
      && is_type_disjoint tenv ty1 ty2)
  then
    Lints_errors.invalid_disjointness_check
      p
      (Utils.strip_ns name)
      (Env.print_ty env ty1)
      (Env.print_ty env ty2)
  else
    let ty1 = Tast_env.strip_dynamic env ty1 in
    let ty2 = Tast_env.strip_dynamic env ty2 in
    if
      Typing_utils.(
        (not (is_nothing tenv ty1))
        && (not (is_nothing tenv ty2))
        && is_type_disjoint tenv ty1 ty2)
    then
      Lints_errors.invalid_disjointness_check_dynamic
        p
        (Utils.strip_ns name)
        (Env.print_ty env ty1)
        (Env.print_ty env ty2)

let rec check_non_disjoint_tyl env p name tyl =
  match tyl with
  | [] -> ()
  | ty :: tyl ->
    List.iter tyl ~f:(check_non_disjoint env p name ty);
    check_non_disjoint_tyl env p name tyl

let has_non_disjoint_attr tp =
  Attributes.mem SN.UserAttributes.uaNonDisjoint tp.tp_user_attributes

let check_function_type_args_non_disjoint env p name tal fun_ty =
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
      check_non_disjoint_tyl env p name tyl
  | _ -> ()

let member_hook env p class_type method_name tal =
  match Cls.get_method class_type method_name with
  | None -> ()
  | Some { ce_type = (lazy fun_ty); _ } ->
    check_function_type_args_non_disjoint env p method_name tal fun_ty

let check_instance_method_call env p receiver_type method_name tal =
  let ctx = Tast_env.get_ctx env in
  Tast_env.get_class_ids env receiver_type
  |> List.filter_map ~f:(fun c ->
         Decl_provider.get_class ctx c |> Decl_entry.to_option)
  |> List.iter ~f:(fun class_type ->
         member_hook env p class_type method_name tal)

let check_static_method_call env p class_result method_name tal =
  match class_result with
  | Decl_entry.Found cd -> begin
    match Cls.get_smethod cd method_name with
    | None -> ()
    | Some { ce_type = (lazy fun_ty); _ } ->
      check_function_type_args_non_disjoint env p method_name tal fun_ty
  end
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Call { func = (_, _, Id (_, name)); targs = _ :: _ as tal; _ })
        -> begin
        match Decl_provider.get_fun (Tast_env.get_ctx env) name with
        | Decl_entry.Found { fe_type; _ } ->
          check_function_type_args_non_disjoint env p name tal fe_type
        | _ -> ()
      end
      | ( _,
          p,
          Call
            {
              func =
                ( _,
                  _,
                  Obj_get
                    ( (receiver_type, _, _),
                      (_, _, Id (_, method_name)),
                      _,
                      Aast.Is_method ) );
              targs = _ :: _ as tal;
              _;
            } ) ->
        check_instance_method_call env p receiver_type method_name tal
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

        check_static_method_call env p class_result method_name tal
      | _ -> ()
  end
