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

let lambda_decl_in_env (env : Decl_env.env) (f : Nast.fun_) :
    Typing_defs.fun_elt =
  let ifc_decl = FunUtils.find_policied_attribute f.f_user_attributes in
  let return_disposable =
    FunUtils.has_return_disposable_attribute f.f_user_attributes
  in
  let ft_readonly_this = Option.is_some f.f_readonly_this in
  let ft_is_memoized = FunUtils.has_memoize_attribute f.f_user_attributes in
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
  let fe_support_dynamic_type =
    Naming_attributes.mem
      SN.UserAttributes.uaSupportDynamicType
      f.f_user_attributes
  in
  let fe_pos = Decl_env.make_decl_pos env @@ f.f_span in
  let fe_type =
    mk
      ( Reason.Rwitness_from_decl fe_pos,
        Tfun
          {
            ft_tparams = [];
            ft_where_constraints = [];
            ft_params = params;
            ft_implicit_params = { capability };
            ft_ret = { et_type = ret_ty; et_enforced = Unenforced };
            ft_flags =
              Typing_defs_flags.Fun.make
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
            (* Lambdas cannot be cross package *)
            ft_cross_package = None;
          } )
  in
  {
    fe_pos;
    fe_module = None;
    fe_internal = false;
    fe_type;
    fe_deprecated;
    fe_php_std_lib;
    fe_support_dynamic_type;
    fe_no_auto_dynamic = false;
    fe_no_auto_likes = false;
  }
