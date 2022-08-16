(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used to convert an AST into a type signature. Produces signatures for
 * functions, global constants, typedefs, and records. Classes are handled in
 * Decl_folded_class (folded decl) or Typing_classes_heap (shallow decl). *)
(*****************************************************************************)

open Hh_prelude
open Aast
open Typing_defs
open Typing_deps
module FunUtils = Decl_fun_utils
module Reason = Typing_reason
module SN = Naming_special_names

(*****************************************************************************)
(* Section declaring the type of a function *)
(*****************************************************************************)

let rec fun_naming_and_decl_DEPRECATED
    (ctx : Provider_context.t) (f : Nast.fun_def) : string * Typing_defs.fun_elt
    =
  let f = Errors.ignore_ (fun () -> Naming.fun_def ctx f) in
  let fe = fun_decl ctx f in
  (snd f.fd_fun.f_name, fe)

and fun_decl (ctx : Provider_context.t) (f : Nast.fun_def) : Typing_defs.fun_elt
    =
  let dep = Dep.Fun (snd f.fd_fun.f_name) in
  let env =
    { Decl_env.mode = f.fd_mode; droot = Some dep; droot_member = None; ctx }
  in
  let module_ = f.fd_module in
  fun_decl_in_env
    env
    ~is_lambda:false
    ~is_internal:f.fd_internal
    f.fd_fun
    module_

and lambda_decl_in_env (env : Decl_env.env) (f : Nast.fun_) :
    Typing_defs.fun_elt =
  fun_decl_in_env env ~is_lambda:true ~is_internal:false f None

and fun_decl_in_env
    (env : Decl_env.env)
    ~(is_lambda : bool)
    ~(is_internal : bool)
    (f : Nast.fun_)
    (module_ : Ast_defs.id option) : Typing_defs.fun_elt =
  let ifc_decl = FunUtils.find_policied_attribute f.f_user_attributes in
  let return_disposable =
    FunUtils.has_return_disposable_attribute f.f_user_attributes
  in
  let ft_readonly_this = Option.is_some f.f_readonly_this in
  let ft_is_memoized = FunUtils.has_memoize_attribute f.f_user_attributes in
  let params = FunUtils.make_params env ~is_lambda f.f_params in
  let (_pos, capability) =
    Decl_hint.aast_contexts_to_decl_capability env f.f_ctxs (fst f.f_name)
  in
  let ret_ty =
    FunUtils.ret_from_fun_kind
      ~is_lambda
      env
      (fst f.f_name)
      f.f_fun_kind
      (hint_of_type_hint f.f_ret)
  in
  let tparams = List.map f.f_tparams ~f:(FunUtils.type_param env) in
  let where_constraints =
    List.map f.f_where_constraints ~f:(FunUtils.where_constraint env)
  in
  let fe_deprecated =
    Naming_attributes_params.deprecated
      ~kind:"function"
      f.f_name
      f.f_user_attributes
  in
  let fe_php_std_lib =
    Naming_attributes.mem SN.UserAttributes.uaPHPStdLib f.f_user_attributes
  in
  let fe_support_dynamic_type =
    Naming_attributes.mem
      SN.UserAttributes.uaSupportDynamicType
      f.f_user_attributes
  in
  let fe_pos = Decl_env.make_decl_pos env @@ fst f.f_name in
  let fe_type =
    mk
      ( Reason.Rwitness_from_decl fe_pos,
        Tfun
          {
            ft_tparams = tparams;
            ft_where_constraints = where_constraints;
            ft_params = params;
            ft_implicit_params = { capability };
            ft_ret = { et_type = ret_ty; et_enforced = Unenforced };
            ft_flags =
              make_ft_flags
                f.f_fun_kind
                ~return_disposable
                ~returns_readonly:(Option.is_some f.f_readonly_ret)
                ~readonly_this:ft_readonly_this
                ~support_dynamic_type:fe_support_dynamic_type
                ~is_memoized:ft_is_memoized
                ~variadic:
                  (List.exists f.f_params ~f:(fun p -> p.param_is_variadic));
            (* TODO: handle const attribute *)
            ft_ifc_decl = ifc_decl;
          } )
  in
  {
    fe_pos;
    fe_module = module_;
    fe_internal = is_internal;
    fe_type;
    fe_deprecated;
    fe_php_std_lib;
    fe_support_dynamic_type;
  }

(*****************************************************************************)
(* Dealing with typedefs *)
(*****************************************************************************)
and typedef_decl (ctx : Provider_context.t) (tdef : Nast.typedef) :
    Typing_defs.typedef_type =
  let {
    t_annotation = ();
    t_name = (name_pos, tid);
    t_tparams = params;
    t_constraint = tcstr;
    t_kind = concrete_type;
    t_user_attributes;
    t_file_attributes = _;
    t_namespace = _;
    t_mode = mode;
    t_vis = td_vis;
    t_span = _;
    t_emit_id = _;
    t_is_ctx = td_is_ctx;
    t_internal = td_internal;
    (* We'll consume this in the next diff *)
    t_module = td_module;
    t_docs_url = td_docs_url;
  } =
    tdef
  in
  let dep = Typing_deps.Dep.Type tid in
  let env = { Decl_env.mode; droot = Some dep; droot_member = None; ctx } in
  let td_tparams = List.map params ~f:(FunUtils.type_param env) in
  let td_type = Decl_hint.hint env concrete_type in
  let td_constraint = Option.map tcstr ~f:(Decl_hint.hint env) in
  let td_pos = Decl_env.make_decl_pos env name_pos in
  let td_attributes =
    List.map
      t_user_attributes
      ~f:(Decl_hint.aast_user_attribute_to_decl_user_attribute env)
  in
  {
    td_module;
    td_vis;
    td_tparams;
    td_constraint;
    td_type;
    td_pos;
    td_is_ctx;
    td_attributes;
    td_internal;
    td_docs_url;
  }

let typedef_naming_and_decl_DEPRECATED
    (ctx : Provider_context.t) (tdef : Nast.typedef) :
    string * Typing_defs.typedef_type =
  let tdef = Errors.ignore_ (fun () -> Naming.typedef ctx tdef) in
  let tdecl = typedef_decl ctx tdef in
  (snd tdef.t_name, tdecl)

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let const_decl (ctx : Provider_context.t) (cst : Nast.gconst) :
    Typing_defs.const_decl =
  let {
    Aast.cst_name = (name_pos, name);
    cst_annotation = _;
    cst_emit_id = _;
    cst_mode;
    cst_namespace = _;
    cst_span;
    cst_type;
    cst_value = (_, value_pos, value);
  } =
    cst
  in
  let dep = Dep.GConst name in
  let env =
    { Decl_env.mode = cst_mode; droot = Some dep; droot_member = None; ctx }
  in
  let cd_type =
    match cst_type with
    | Some h -> Decl_hint.hint env h
    | None ->
      (match Decl_utils.infer_const value with
      | Some tprim ->
        mk
          ( Reason.Rwitness_from_decl (Decl_env.make_decl_pos env value_pos),
            Tprim tprim )
      (* A NAST check will take care of rejecting constants that have neither
       * an initializer nor a literal initializer *)
      | None ->
        mk
          ( Reason.Rwitness_from_decl (Decl_env.make_decl_pos env name_pos),
            Typing_defs.make_tany () ))
  in
  { cd_pos = Decl_env.make_decl_pos env cst_span; cd_type }

let const_naming_and_decl_DEPRECATED
    (ctx : Provider_context.t) (cst : Nast.gconst) :
    string * Typing_defs.const_decl =
  let cst = Errors.ignore_ (fun () -> Naming.global_const ctx cst) in
  let hint_ty = const_decl ctx cst in
  (snd cst.cst_name, hint_ty)

(*****************************************************************************)
(* Modules *)
(*****************************************************************************)

let module_reference_decl (name : Aast.md_name_kind) =
  match name with
  | MDNameGlobal _ -> MRGlobal
  | MDNameExact (_, s) -> MRExact s
  | MDNamePrefix (_, s) -> MRPrefix s

let module_naming_and_decl_DEPRECATED
    (_ : Provider_context.t) (md : Nast.module_def) :
    string * Typing_defs.module_def_type =
  ( snd md.md_name,
    {
      mdt_pos = Pos_or_decl.of_raw_pos @@ fst md.md_name;
      mdt_exports = List.map md.md_exports ~f:module_reference_decl;
      mdt_imports = List.map md.md_imports ~f:module_reference_decl;
    } )
