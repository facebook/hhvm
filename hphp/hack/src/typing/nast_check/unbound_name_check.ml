(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Checks to determine whether names referenced in a file are defined globally.
 *
 * NOTE: Unlike other nast checks, this one depends on and also
 * modifies global naming table state. We would rather have this done in typing
 * but currently there are multiple scenarios when the typechecker
 * does not comprehensively check expressions. We are then left with
 * an unrecorded dependency. This should be fixed on some more basic level.
 *)

open Hh_prelude

type env = {
  droot: Typing_deps.Dep.dependent Typing_deps.Dep.variant;
  ctx: Provider_context.t;
  type_params: Aast.reify_kind SMap.t;
  (* Need some context to differentiate global consts and other Id's *)
  seen_names: Pos.t SMap.t;
  (* Contexts where typedefs are valid typenames *)
  class_id_allow_typedef: bool;
  hint_allow_typedef: bool;
  hint_context: Name_context.t;
}

let get_custom_error_config env =
  let tc_opt = Provider_context.get_tcopt env.ctx in
  TypecheckerOptions.custom_error_config tc_opt

let handle_unbound_name env custom_err_config (pos, name) kind =
  (* We've already errored in naming if we get *Unknown* class *)
  if String.equal name Naming_special_names.Classes.cUnknown then
    ()
  else begin
    let tcopt = Provider_context.get_tcopt env.ctx in
    let warn =
      match TypecheckerOptions.repo_stdlib_path tcopt with
      | None -> false
      | Some prefix ->
        let file_path = Relative_path.suffix (Pos.filename pos) in
        String.is_prefix file_path ~prefix
    in
    if warn then
      Typing_warning_utils.add_
        tcopt
        Typing_warning.
          ( pos,
            Unbound_name_warning,
            {
              Unbound_name_warning.name;
              kind_str = Naming_error_utils.unbound_name_kind kind;
            } )
    else
      Diagnostics.add_diagnostic
        (Naming_error_utils.to_user_diagnostic
           (Naming_error.Unbound_name { pos; name; kind })
           custom_err_config);
    (* In addition to reporting errors, we also add to the global dependency table *)
    let dep =
      let open Name_context in
      match kind with
      | FunctionNamespace -> Typing_deps.Dep.Fun name
      | TypeNamespace -> Typing_deps.Dep.Type name
      | ConstantNamespace -> Typing_deps.Dep.GConst name
      | TraitContext -> Typing_deps.Dep.Type name
      | ClassContext -> Typing_deps.Dep.Type name
      | ModuleNamespace -> Typing_deps.Dep.Module name
      | PackageNamespace ->
        failwith "impossible match case" (* TODO (T148526825) *)
    in
    Typing_deps.add_idep (Provider_context.get_deps_mode env.ctx) env.droot dep
  end

let has_canon_name env custom_err_config get_name get_pos (pos, name) =
  match get_name env.ctx name with
  | None -> false
  | Some suggest_name -> begin
    match get_pos env.ctx suggest_name with
    | None -> false
    | Some suggest_pos ->
      Diagnostics.add_diagnostic
        (Naming_error_utils.to_user_diagnostic
           (Naming_error.Did_you_mean { pos; name; suggest_pos; suggest_name })
           custom_err_config);
      true
  end

let check_fun_name env custom_err_config ((_, name) as id) =
  if Naming_special_names.PreNamespacedFunctions.is_pre_namespaced_function name
  then
    ()
  else if Naming_provider.fun_exists env.ctx name then
    ()
  else if
    has_canon_name
      env
      custom_err_config
      Naming_provider.get_fun_canon_name
      Naming_global.GEnv.fun_pos
      id
  then
    ()
  else
    handle_unbound_name env custom_err_config id Name_context.FunctionNamespace

let check_const_name env custom_err_config ((_, name) as id) =
  if Naming_provider.const_exists env.ctx name then
    ()
  else
    handle_unbound_name env custom_err_config id Name_context.ConstantNamespace

let check_module_name env custom_err_config ((_, name) as id) =
  if Naming_provider.module_exists env.ctx name then
    ()
  else
    handle_unbound_name env custom_err_config id Name_context.ModuleNamespace

let check_module_if_present env custom_err_config id_opt =
  Option.iter id_opt ~f:(check_module_name env custom_err_config)

let check_package_name env custom_err_config (pos, name) =
  if
    PackageInfo.package_exists (Provider_context.get_package_info env.ctx) name
    || not
         (TypecheckerOptions.check_packages
            (Provider_context.get_tcopt env.ctx))
  then
    ()
  else
    Diagnostics.add_diagnostic
      (Naming_error_utils.to_user_diagnostic
         (Naming_error.Unbound_name
            { pos; name; kind = Name_context.PackageNamespace })
         custom_err_config)

let check_type_name
    ?(kind = Name_context.TypeNamespace)
    env
    custom_err_config
    ((pos, name) as id)
    ~allow_typedef
    ~allow_generics =
  if String.equal name Naming_special_names.Classes.cHH_BuiltinEnum then
    ()
  else
    match SMap.find_opt name env.type_params with
    | Some reified ->
      (* TODO: These throw typing errors instead of naming errors *)
      if not allow_generics then
        Diagnostics.add_diagnostic
          Nast_check_error.(to_user_diagnostic @@ Generics_not_allowed pos);
      begin
        match reified with
        | Aast.Erased ->
          Diagnostics.add_diagnostic
            Nast_check_error.(
              to_user_diagnostic
              @@ Generic_at_runtime { pos; prefix = "Erased" })
        | Aast.SoftReified ->
          Diagnostics.add_diagnostic
            Nast_check_error.(
              to_user_diagnostic
              @@ Generic_at_runtime { pos; prefix = "Soft reified" })
        | Aast.Reified -> ()
      end
    | None -> begin
      match Naming_provider.get_type_kind env.ctx name with
      | Some Naming_types.TTypedef when not allow_typedef ->
        let def_pos =
          Naming_provider.get_type_pos env.ctx name |> Option.value_exn
        in
        let (decl_pos, _) =
          Naming_global.GEnv.get_type_full_pos env.ctx (def_pos, name)
        in
        Diagnostics.add_diagnostic
          (Naming_error_utils.to_user_diagnostic
             (Naming_error.Unexpected_typedef
                { pos; decl_pos; expected_kind = kind })
             custom_err_config)
      | Some _ -> ()
      | None ->
        if
          has_canon_name
            env
            custom_err_config
            Naming_provider.get_type_canon_name
            Naming_global.GEnv.type_pos
            id
        then
          ()
        else
          handle_unbound_name env custom_err_config id kind
    end

let check_type_hint
    ?(kind = Name_context.TypeNamespace)
    env
    custom_err_config
    ((_, name) as id)
    ~allow_typedef
    ~allow_generics =
  if String.equal name Naming_special_names.Typehints.wildcard then
    ()
  else
    check_type_name
      ~kind
      env
      custom_err_config
      id
      ~allow_typedef
      ~allow_generics

let extend_type_params init paraml =
  List.fold_right
    ~init
    ~f:(fun { Aast.tp_name = (_, name); tp_reified; _ } acc ->
      SMap.add name tp_reified acc)
    paraml

let handler ctx =
  object
    inherit [env] Stateful_aast_visitor.default_nast_visitor_with_state as super

    (* The following are all setting the environments / context correctly *)
    method initial_state =
      {
        droot = Typing_deps.Dep.Fun "";
        ctx;
        type_params = SMap.empty;
        seen_names = SMap.empty;
        class_id_allow_typedef = false;
        hint_allow_typedef = true;
        hint_context = Name_context.TypeNamespace;
      }

    method! at_class_ env c =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Type (snd c.Aast.c_name);
          type_params = extend_type_params SMap.empty c.Aast.c_tparams;
        }
      in
      let custom_err_config = get_custom_error_config env in
      check_module_if_present new_env custom_err_config c.Aast.c_module;
      new_env

    method! at_typedef env td =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Type (snd td.Aast.t_name);
          type_params = extend_type_params SMap.empty td.Aast.t_tparams;
        }
      in
      let custom_err_config = get_custom_error_config env in
      check_module_if_present new_env custom_err_config td.Aast.t_module;
      new_env

    method! at_fun_def env fd =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Fun (snd fd.Aast.fd_name);
          type_params = extend_type_params env.type_params fd.Aast.fd_tparams;
        }
      in
      let custom_err_config = get_custom_error_config env in
      check_module_if_present new_env custom_err_config fd.Aast.fd_module;
      new_env

    method! at_gconst env gconst =
      let new_env =
        { env with droot = Typing_deps.Dep.GConst (snd gconst.Aast.cst_name) }
      in
      let custom_err_config = get_custom_error_config env in
      check_module_if_present new_env custom_err_config gconst.Aast.cst_module;
      new_env

    method! at_method_ env m =
      {
        env with
        type_params = extend_type_params env.type_params m.Aast.m_tparams;
      }

    method! at_targ env _ = { env with hint_allow_typedef = true }

    method! at_class_hint env _ =
      {
        env with
        hint_context = Name_context.ClassContext;
        hint_allow_typedef = false;
      }

    method! at_trait_hint env _ =
      {
        env with
        hint_context = Name_context.TraitContext;
        hint_allow_typedef = false;
      }

    method! at_xhp_attr_hint env _ = { env with hint_allow_typedef = false }

    (* Below are the methods where we check for unbound names *)
    method! at_expr env (_, _, e) =
      match e with
      | Aast.FunctionPointer (Aast.FP_id ((p, name) as id), _, _)
      | Aast.(Call { func = (_, _, Aast.Id ((p, name) as id)); _ }) ->
        let custom_err_config = get_custom_error_config env in
        let () = check_fun_name env custom_err_config id in
        { env with seen_names = SMap.add name p env.seen_names }
      | Aast.Id ((p, name) as id) ->
        let custom_err_config = get_custom_error_config env in
        let () =
          match SMap.find_opt name env.seen_names with
          | None -> check_const_name env custom_err_config id
          | Some pos when not @@ Pos.equal p pos ->
            check_const_name env custom_err_config id
          | _ -> ()
        in
        env
      | Aast.Method_caller (id, _)
      | Aast.Xml (id, _, _) ->
        let custom_err_config = get_custom_error_config env in
        let () =
          check_type_name
            env
            custom_err_config
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Name_context.ClassContext
            id
        in
        env
      | Aast.Class_const ((_, _, Aast.CI _), (_, s)) when String.equal s "class"
        ->
        { env with class_id_allow_typedef = true }
      | Aast.Nameof _ -> { env with class_id_allow_typedef = true }
      | Aast.Obj_get (_, (_, _, Aast.Id (p, name)), _, _) ->
        { env with seen_names = SMap.add name p env.seen_names }
      | Aast.EnumClassLabel (Some cname, _) ->
        let allow_typedef = (* we might reconsider this ? *) false in
        let custom_err_config = get_custom_error_config env in
        let () =
          check_type_name
            env
            custom_err_config
            ~allow_typedef
            ~allow_generics:false
            ~kind:Name_context.ClassContext
            cname
        in
        env
      | Aast.Package pkg ->
        let custom_err_config = get_custom_error_config env in
        let () = check_package_name env custom_err_config pkg in
        env
      | _ -> env

    method! at_shape_field_name env sfn =
      let () =
        match sfn with
        | Ast_defs.SFclass_const (id, _) ->
          let custom_err_config = get_custom_error_config env in
          check_type_name
            env
            custom_err_config
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Name_context.ClassContext
            id
        | _ -> ()
      in
      env

    method! at_user_attribute env { Aast.ua_name; Aast.ua_params; _ } =
      let () =
        if not @@ Naming_special_names.UserAttributes.is_reserved (snd ua_name)
        then
          let custom_err_config = get_custom_error_config env in
          check_type_name
            env
            custom_err_config
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Name_context.ClassContext
            ua_name
      in
      let () =
        if
          String.equal
            (snd ua_name)
            Naming_special_names.UserAttributes.uaRequirePackage
        then
          List.iter
            ~f:(function
              | (_, pos, Aast.String pkg_name) ->
                let custom_err_config = get_custom_error_config env in
                check_package_name env custom_err_config (pos, pkg_name)
              | _ -> ())
            ua_params
      in
      let () =
        if
          String.equal
            (snd ua_name)
            Naming_special_names.UserAttributes.uaPackageOverride
        then
          List.iter
            ~f:(function
              | (_, pos, Aast.String pkg_name) ->
                let custom_err_config = get_custom_error_config env in
                check_package_name env custom_err_config (pos, pkg_name)
              | _ -> ())
            ua_params
      in
      env

    method! at_class_id env (_, _, ci) =
      match ci with
      | Aast.CI id ->
        let custom_err_config = get_custom_error_config env in
        let () =
          check_type_name
            env
            custom_err_config
            ~allow_typedef:env.class_id_allow_typedef
            ~allow_generics:true
            ~kind:Name_context.ClassContext
            id
        in
        env
      | _ -> env

    method! at_catch env (id, _, _) =
      let custom_err_config = get_custom_error_config env in
      let () =
        check_type_name
          env
          custom_err_config
          ~allow_typedef:false
          ~allow_generics:false
          ~kind:Name_context.ClassContext
          id
      in
      env

    method! at_hint env h =
      match snd h with
      | Aast.Hfun Aast_defs.{ hf_tparams; _ } ->
        let env =
          let type_params =
            List.fold_left
              hf_tparams
              ~init:env.type_params
              ~f:(fun acc Aast_defs.{ htp_name = (_, nm); _ } ->
                SMap.add nm Aast_defs.Erased acc)
          in
          { env with type_params }
        in
        super#at_hint env h
      | Aast.Happly (id, _) ->
        let custom_err_config = get_custom_error_config env in
        let () =
          check_type_hint
            env
            custom_err_config
            ~allow_typedef:env.hint_allow_typedef
            ~allow_generics:false
            ~kind:env.hint_context
            id
        in
        (* Intentionally set allow_typedef to true for a hint's type parameters *
         * because there are no runtime restrictions *)
        { env with hint_allow_typedef = true }
      | _ -> env
  end
