(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used to convert an AST into a type signature. Historically, all
 * decls were produced this way. Now, they are produced only by the direct decl
 * parser. Only lambda type signatures are produced from ASTs here. *)
(*****************************************************************************)

open Hh_prelude
open Aast
open Typing_defs
module FunUtils = Decl_fun_utils
module Reason = Typing_reason
module SN = Naming_special_names

let lambda_flags (f : Nast.fun_) =
  let return_disposable =
    FunUtils.has_return_disposable_attribute f.f_user_attributes
  in
  let ft_readonly_this = Option.is_some f.f_readonly_this in
  let ft_is_memoized = FunUtils.has_memoize_attribute f.f_user_attributes in
  let fe_support_dynamic_type =
    Naming_attributes.mem
      SN.UserAttributes.uaSupportDynamicType
      f.f_user_attributes
  in
  Typing_defs_flags.Fun.make
    f.f_fun_kind
    ~return_disposable
    ~returns_readonly:(Option.is_some f.f_readonly_ret)
    ~readonly_this:ft_readonly_this
    ~support_dynamic_type:fe_support_dynamic_type
    ~is_memoized:ft_is_memoized
    ~variadic:(List.exists f.f_params ~f:Aast_utils.is_param_variadic)

let lambda_decl_in_env (env : Decl_env.env) (f : Nast.fun_) :
    Typing_defs.fun_elt =
  let ft_tparams =
    List.map
      f.f_tparams
      ~f:(fun Aast_defs.{ htp_name; htp_constraints; htp_user_attributes } ->
        let tp_name =
          let (pos, name) = htp_name in
          (Pos_or_decl.of_raw_pos pos, name)
        and tp_constraints =
          List.map htp_constraints ~f:(fun (kind, hint) ->
              (kind, Decl_hint.hint env hint))
        and tp_user_attributes =
          List.map htp_user_attributes ~f:(fun (pos, id) ->
              let ua_name = (Pos_or_decl.of_raw_pos pos, id) in
              Typing_defs_core.{ ua_name; ua_params = []; ua_raw_val = None })
        in
        Typing_defs_core.
          {
            tp_name;
            tp_constraints;
            tp_user_attributes;
            tp_variance = Ast_defs.Invariant;
            tp_reified = Ast_defs.Erased;
          })
  in
  let ft_instantiated = List.is_empty ft_tparams in
  let params = FunUtils.make_params env f.f_params in
  let (_pos, capability) =
    Decl_hint.aast_contexts_to_decl_capability env f.f_ctxs f.f_span
  in
  let ret_ty =
    FunUtils.ret_from_fun_kind
      env
      f.f_span
      f.f_fun_kind
      (hint_of_type_hint f.f_ret)
  in
  let fe_deprecated =
    Naming_attributes_params.deprecated
      ~kind:"function"
      (f.f_span, ";anonymous")
      f.f_user_attributes
  in
  let fe_php_std_lib =
    Naming_attributes.mem SN.UserAttributes.uaPHPStdLib f.f_user_attributes
  in
  let fe_pos = Decl_env.make_decl_pos env @@ f.f_span in
  let ft_flags = lambda_flags f in
  let fe_type =
    mk
      ( Reason.witness_from_decl fe_pos,
        Tfun
          {
            ft_tparams;
            ft_where_constraints = [];
            ft_params = params;
            ft_implicit_params = { capability };
            ft_ret = ret_ty;
            ft_flags;
            (* TODO: handle const attribute *)
            ft_instantiated;
          } )
  in
  {
    fe_pos;
    fe_module = None;
    fe_package = None;
    fe_internal = false;
    fe_type;
    fe_deprecated;
    fe_php_std_lib;
    fe_support_dynamic_type =
      Typing_defs_flags.Fun.support_dynamic_type ft_flags;
    fe_no_auto_dynamic = false;
    fe_no_auto_likes = false;
    (* Lambdas cannot be require package *)
    fe_package_requirement = RPNormal;
  }
