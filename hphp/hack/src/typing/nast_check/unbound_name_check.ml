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
  mode: FileInfo.mode;
  ctx: Provider_context.t;
  type_params: Aast.reify_kind SMap.t;
  (* Need some context to differentiate global consts and other Id's *)
  seen_names: Pos.t SMap.t;
  (* Contexts where typedefs are valid typenames *)
  class_id_allow_typedef: bool;
  hint_allow_typedef: bool;
  hint_context: Errors.name_context;
}

let handle_unbound_name env (pos, name) kind =
  (* We've already errored in naming if we get *Unknown* class *)
  if String.equal name Naming_special_names.Classes.cUnknown then
    ()
  else
    match env.mode with
    | FileInfo.Mstrict
    | FileInfo.Mpartial
    | FileInfo.Mhhi ->
      Errors.unbound_name pos name kind;
      (* In addition to reporting errors, we also add to the global dependency table *)
      let dep =
        match kind with
        | Errors.FunctionNamespace -> Typing_deps.Dep.Fun name
        | Errors.TypeNamespace -> Typing_deps.Dep.Type name
        | Errors.ConstantNamespace -> Typing_deps.Dep.GConst name
        | Errors.TraitContext -> Typing_deps.Dep.Type name
        | Errors.RecordContext -> Typing_deps.Dep.Type name
        | Errors.ClassContext -> Typing_deps.Dep.Type name
      in
      Typing_deps.add_idep
        (Provider_context.get_deps_mode env.ctx)
        env.droot
        dep

let has_canon_name env get_name get_pos (pos, name) =
  match get_name env.ctx name with
  | None -> false
  | Some canon_name ->
    begin
      match get_pos env.ctx canon_name with
      | None -> false
      | Some canon_pos ->
        Errors.did_you_mean_naming pos name canon_pos canon_name;
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
      Naming_global.GEnv.fun_canon_name
      Naming_global.GEnv.fun_pos
      id
  then
    ()
  else
    handle_unbound_name env id Errors.FunctionNamespace

let check_const_name env ((_, name) as id) =
  if Naming_provider.const_exists env.ctx name then
    ()
  else
    handle_unbound_name env id Errors.ConstantNamespace

let check_type_name
    ?(kind = Errors.TypeNamespace)
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
      if not allow_generics then Errors.generics_not_allowed pos;
      begin
        match reified with
        | Aast.Erased -> Errors.generic_at_runtime pos "Erased"
        | Aast.SoftReified -> Errors.generic_at_runtime pos "Soft reified"
        | Aast.Reified -> ()
      end
    | None ->
      begin
        match Naming_provider.get_type_pos_and_kind env.ctx name with
        | Some (def_pos, Naming_types.TTypedef) when not allow_typedef ->
          let (full_pos, _) =
            Naming_global.GEnv.get_type_full_pos env.ctx (def_pos, name)
          in
          Errors.unexpected_typedef pos full_pos kind
        | Some _ -> ()
        | None ->
          if
            has_canon_name
              env
              Naming_global.GEnv.type_canon_name
              Naming_global.GEnv.type_pos
              id
          then
            ()
          else
            handle_unbound_name env id kind
      end

let check_type_hint
    ?(kind = Errors.TypeNamespace)
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
        mode = FileInfo.Mpartial;
        droot = Typing_deps.Dep.Fun "";
        ctx;
        type_params = SMap.empty;
        seen_names = SMap.empty;
        class_id_allow_typedef = false;
        hint_allow_typedef = true;
        hint_context = Errors.TypeNamespace;
      }

    method! at_class_ env c =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Type (snd c.Aast.c_name);
          mode = c.Aast.c_mode;
          type_params = extend_type_params SMap.empty c.Aast.c_tparams;
        }
      in
      new_env

    method! at_typedef env td =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Type (snd td.Aast.t_name);
          mode = FileInfo.Mstrict;
          type_params = extend_type_params SMap.empty td.Aast.t_tparams;
        }
      in
      new_env

    method! at_fun_def env fd =
      let f = fd.Aast.fd_fun in
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.Fun (snd f.Aast.f_name);
          mode = fd.Aast.fd_mode;
          type_params = extend_type_params env.type_params f.Aast.f_tparams;
        }
      in
      new_env

    method! at_gconst env gconst =
      let new_env =
        {
          env with
          droot = Typing_deps.Dep.GConst (snd gconst.Aast.cst_name);
          mode = gconst.Aast.cst_mode;
        }
      in
      new_env

    method! at_file_attribute env _ =
      let new_env =
        { env with droot = Typing_deps.Dep.Fun ""; type_params = SMap.empty }
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
        hint_context = Errors.ClassContext;
        hint_allow_typedef = false;
      }

    method! at_trait_hint env _ =
      {
        env with
        hint_context = Errors.TraitContext;
        hint_allow_typedef = false;
      }

    method! at_record_hint env _ =
      {
        env with
        hint_context = Errors.RecordContext;
        hint_allow_typedef = false;
      }

    method! at_xhp_attr_hint env _ = { env with hint_allow_typedef = false }

    (* Below are the methods where we check for unbound names *)
    method! at_expr env e =
      match snd e with
      | Aast.FunctionPointer (Aast.FP_id ((p, name) as id), _)
      | Aast.Call ((_, Aast.Id ((p, name) as id)), _, _, _) ->
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
      | Aast.Fun_id id ->
        let () = check_fun_name env id in
        env
      | Aast.Method_caller (id, _)
      | Aast.Xml (id, _, _) ->
        let () =
          check_type_name
            env
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Errors.ClassContext
            id
        in
        env
      | Aast.Record (id, _) ->
        let () =
          check_type_name
            env
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Errors.RecordContext
            id
        in
        env
      | Aast.Class_const ((_, Aast.CI _), (_, s)) when String.equal s "class" ->
        { env with class_id_allow_typedef = true }
      | Aast.Obj_get (_, (_, Aast.Id (p, name)), _, _) ->
        { env with seen_names = SMap.add name p env.seen_names }
      | Aast.EnumClassLabel (Some cname, _) ->
        let allow_typedef = (* we might reconsider this ? *) false in
        let () =
          check_type_name
            env
            ~allow_typedef
            ~allow_generics:false
            ~kind:Errors.ClassContext
            cname
        in
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
            ~kind:Errors.ClassContext
            id
        | _ -> ()
      in
      env

    method! at_user_attribute env { Aast.ua_name; _ } =
      let () =
        if not @@ Naming_special_names.UserAttributes.is_reserved (snd ua_name)
        then
          check_type_name
            env
            ~allow_typedef:false
            ~allow_generics:false
            ~kind:Errors.ClassContext
            ua_name
      in
      env

    method! at_class_id env ci =
      match snd ci with
      | Aast.CI id ->
        let () =
          check_type_name
            env
            ~allow_typedef:env.class_id_allow_typedef
            ~allow_generics:true
            ~kind:Errors.ClassContext
            id
        in
        env
      | _ -> env

    method! at_catch env (id, _, _) =
      let () =
        check_type_name
          env
          ~allow_typedef:true
          ~allow_generics:false
          ~kind:Errors.ClassContext
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
