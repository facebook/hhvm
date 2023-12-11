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

let handle_unbound_name env (pos, name) kind =
  (* We've already errored in naming if we get *Unknown* class *)
  if String.equal name Naming_special_names.Classes.cUnknown then
    ()
  else begin
    Errors.add_error
      Naming_error.(to_user_error @@ Unbound_name { pos; name; kind });
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

let has_canon_name env get_name get_pos (pos, name) =
  match get_name env.ctx name with
  | None -> false
  | Some suggest_name -> begin
    match get_pos env.ctx suggest_name with
    | None -> false
    | Some suggest_pos ->
      Errors.add_error
        Naming_error.(
          to_user_error @@ Did_you_mean { pos; name; suggest_pos; suggest_name });
      true
  end

let check_fun_name env ((_, name) as id) =
  if Naming_special_names.SpecialFunctions.is_special_function name then
    ()
  else if Naming_provider.fun_exists env.ctx name then
    ()
  else if
    has_canon_name
      env
      Naming_provider.get_fun_canon_name
      Naming_global.GEnv.fun_pos
      id
  then
    ()
  else
    handle_unbound_name env id Name_context.FunctionNamespace

let check_const_name env ((_, name) as id) =
  if Naming_provider.const_exists env.ctx name then
    ()
  else
    handle_unbound_name env id Name_context.ConstantNamespace

let check_module_name env ((_, name) as id) =
  if Naming_provider.module_exists env.ctx name then
    ()
  else
    handle_unbound_name env id Name_context.ModuleNamespace

let check_module_if_present env id_opt =
  Option.iter id_opt ~f:(check_module_name env)

let check_package_name env (pos, name) =
  if PackageInfo.package_exists (Provider_context.get_package_info env.ctx) name
  then
    ()
  else
    Errors.add_error
      Naming_error.(
        to_user_error
        @@ Unbound_name { pos; name; kind = Name_context.PackageNamespace })

let check_type_name
    ?(kind = Name_context.TypeNamespace)
    env
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
        Errors.add_error
          Nast_check_error.(to_user_error @@ Generics_not_allowed pos);
      begin
        match reified with
        | Aast.Erased ->
          Errors.add_error
            Nast_check_error.(
              to_user_error @@ Generic_at_runtime { pos; prefix = "Erased" })
        | Aast.SoftReified ->
          Errors.add_error
            Nast_check_error.(
              to_user_error
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
        Errors.add_error
          Naming_error.(
            to_user_error
            @@ Unexpected_typedef { pos; decl_pos; expected_kind = kind })
      | Some _ -> ()
      | None ->
        if
          has_canon_name
            env
            Naming_provider.get_type_canon_name
            Naming_global.GEnv.type_pos
            id
        then
          ()
        else
          handle_unbound_name env id kind
    end

let check_type_hint
    ?(kind = Name_context.TypeNamespace)
    env
    ((_, name) as id)
    ~allow_typedef
    ~allow_generics =
  if String.equal name Naming_special_names.Typehints.wildcard then
    ()
  else
    check_type_name ~kind env id ~allow_typedef ~allow_generics

let extend_type_params init paraml =
  List.fold_right
    ~init
    ~f:(fun { Aast.tp_name = (_, name); tp_reified; _ } acc ->
      SMap.add name tp_reified acc)
    paraml

let handler ctx =
  object
    inherit [env] Stateful_aast_visitor.default_nast_visitor_with_state

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
      check_module_if_present new_env c.Aast.c_module;
      new_env

    method! at_typedef env td =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Type (snd td.Aast.t_name);
          type_params = extend_type_params SMap.empty td.Aast.t_tparams;
        }
      in
      check_module_if_present new_env td.Aast.t_module;
      new_env

    method! at_fun_def env fd =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Fun (snd fd.Aast.fd_name);
          type_params = extend_type_params env.type_params fd.Aast.fd_tparams;
        }
      in
      check_module_if_present new_env fd.Aast.fd_module;
      new_env

    method! at_gconst env gconst =
      let new_env =
        { env with droot = Typing_deps.Dep.GConst (snd gconst.Aast.cst_name) }
      in
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
      | Aast.FunctionPointer (Aast.FP_id ((p, name) as id), _)
      | Aast.(Call { func = (_, _, Aast.Id ((p, name) as id)); _ }) ->
        let () = check_fun_name env id in
        { env with seen_names = SMap.add name p env.seen_names }
      | Aast.Id ((p, name) as id) ->
        let () =
          match SMap.find_opt name env.seen_names with
          | None -> check_const_name env id
          | Some pos when not @@ Pos.equal p pos -> check_const_name env id
          | _ -> ()
        in
        env
      | Aast.Method_caller (id, _)
      | Aast.Xml (id, _, _) ->
        let () =
          check_type_name
            env
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
        let () =
          check_type_name
            env
            ~allow_typedef
            ~allow_generics:false
            ~kind:Name_context.ClassContext
            cname
        in
        env
      | Aast.Package pkg ->
        let () = check_package_name env pkg in
        env
      | _ -> env

    method! at_shape_field_name env sfn =
      let () =
        match sfn with
        | Ast_defs.SFclass_const (id, _) ->
          check_type_name
            env
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
          check_type_name
            env
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Name_context.ClassContext
            ua_name
      in
      let () =
        if
          String.equal
            (snd ua_name)
            Naming_special_names.UserAttributes.uaCrossPackage
        then
          List.iter
            ~f:(function
              | (_, pos, Aast.String pkg_name) ->
                check_package_name env (pos, pkg_name)
              | _ -> ())
            ua_params
      in
      env

    method! at_class_id env (_, _, ci) =
      match ci with
      | Aast.CI id ->
        let () =
          check_type_name
            env
            ~allow_typedef:env.class_id_allow_typedef
            ~allow_generics:true
            ~kind:Name_context.ClassContext
            id
        in
        env
      | _ -> env

    method! at_catch env (id, _, _) =
      let () =
        check_type_name
          env
          ~allow_typedef:false
          ~allow_generics:false
          ~kind:Name_context.ClassContext
          id
      in
      env

    method! at_hint env h =
      match snd h with
      | Aast.Happly (id, _) ->
        let () =
          check_type_hint
            env
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
