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
        Typing_warning.Non_disjoint_check.name = Utils.strip_ns name;
        dynamic;
        ty1 = Env.print_ty env ty1;
        ty2 = Env.print_ty env ty2;
      } )

let check_non_disjoint ~is_dynamic_call ~as_lint env p name ty1 ty2 =
  match Env.is_disjoint ~is_dynamic_call env ty1 ty2 with
  | Env.NonDisjoint -> ()
  | Env.DisjointIgnoringDynamic (ty1, ty2) ->
    add_warning env p name ty1 ty2 ~dynamic:true ~as_lint
  | Env.Disjoint ->
    add_warning env p name ty1 ty2 ~dynamic:is_dynamic_call ~as_lint

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

let is_in_overlapping tp overlapping =
  match overlapping with
  | None -> false
  | Some s -> SSet.mem (snd tp.tp_name) s

let check_function_type_args_non_disjoint
    ~is_dynamic_call
    ~as_lint
    env
    p
    class_typarams
    name
    overlapping
    (class_tyargs : Tast.ty list)
    (method_tyargs : Tast.ty list)
    fun_ty =
  match get_node fun_ty with
  | Tfun { ft_tparams = method_typarams; _ } ->
    if
      List.exists method_typarams ~f:has_non_disjoint_attr
      || Option.is_some overlapping
    then
      let (method_pairs, _) =
        List.zip_with_remainder method_typarams method_tyargs
      in
      let (class_pairs, _) =
        List.zip_with_remainder class_typarams class_tyargs
      in
      let tyl =
        List.filter_map (class_pairs @ method_pairs) ~f:(fun (tp, ty) ->
            if has_non_disjoint_attr tp || is_in_overlapping tp overlapping then
              Some ty
            else
              None)
      in
      check_non_disjoint_tyl ~is_dynamic_call ~as_lint env p name tyl
  | _ -> ()

let get_origin_info env class_id class_type class_tyargs origin =
  if String.equal origin class_id then
    Some (Cls.tparams class_type, class_tyargs)
  else
    match
      Decl_provider.get_class (Tast_env.get_ctx env) origin
      |> Decl_entry.to_option
    with
    | None -> None
    | Some origin_class_type ->
      (match Cls.get_ancestor class_type origin with
      | Some ty -> begin
        match get_node ty with
        | Tapply (_, tyargs) ->
          let ety_env =
            {
              empty_expand_env with
              substs =
                Typing_utils.make_locl_subst_for_class_tparams
                  origin_class_type
                  class_tyargs;
            }
          in
          let origin_class_tyargs =
            List.map tyargs ~f:(fun ty ->
                snd (Tast_env.localize env ety_env ty))
          in
          Some (Cls.tparams origin_class_type, origin_class_tyargs)
        | _ -> None
      end
      | None -> None)

let member_hook
    ~is_dynamic_call ~as_lint env p class_id class_tyargs method_name tal =
  match
    Decl_provider.get_class (Tast_env.get_ctx env) class_id
    |> Decl_entry.to_option
  with
  | None -> ()
  | Some class_type ->
    (match Cls.get_method class_type method_name with
    | None -> ()
    | Some { ce_type = (lazy fun_ty); ce_overlapping_tparams; ce_origin; _ } ->
      (match get_origin_info env class_id class_type class_tyargs ce_origin with
      | None -> ()
      | Some (origin_class_typarams, origin_class_tyargs) ->
        check_function_type_args_non_disjoint
          ~is_dynamic_call
          ~as_lint
          env
          p
          origin_class_typarams
          method_name
          ce_overlapping_tparams
          origin_class_tyargs
          (List.map tal ~f:fst)
          fun_ty))

let check_instance_method_call
    env p ~as_lint function_type receiver_type method_name tal =
  let tenv = Env.tast_env_as_typing_env env in
  let is_dynamic_call =
    Option.is_some
      (snd (Typing_dynamic_utils.try_strip_dynamic tenv function_type))
  in
  Tast_env.get_receiver_ids env receiver_type
  |> List.iter ~f:(fun ri ->
         match ri with
         | Tast_env.RIclass (cid, tyargs) ->
           member_hook
             ~is_dynamic_call
             ~as_lint
             env
             p
             cid
             tyargs
             method_name
             tal
         | _ -> ())

let check_static_method_call env p ~as_lint class_result method_name tal =
  match class_result with
  | Decl_entry.Found cd -> begin
    match Cls.get_smethod cd method_name with
    | None -> ()
    | Some { ce_type = (lazy fun_ty); ce_overlapping_tparams; _ } ->
      check_function_type_args_non_disjoint
        ~is_dynamic_call:false
        ~as_lint
        env
        p
        []
        method_name
        ce_overlapping_tparams
        []
        (List.map tal ~f:fst)
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
            []
            name
            None
            []
            (List.map tal ~f:fst)
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
