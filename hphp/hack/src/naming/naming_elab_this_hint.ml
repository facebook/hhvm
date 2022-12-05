(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = {
    (* `this` is forbidden as a hint in this context *)
    forbid_this: bool;
  }

  let empty = { forbid_this = false }

  let create ?(forbid_this = false) () = { forbid_this }
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    (* -- Classes ----------------------------------------------------------- *)
    method! on_class_ env c =
      let (c_tparams, tparams_err) =
        let env = Env.create ~forbid_this:true () in
        super#on_list self#on_tparam env c.Aast.c_tparams
      in

      let (c_extends, extends_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_list self#on_hint env c.Aast.c_extends
      in

      let (c_uses, uses_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_list self#on_hint env c.Aast.c_uses
      in

      let (c_xhp_attrs, xhp_attrs_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_list super#on_xhp_attr env c.Aast.c_xhp_attrs
      in

      let (c_xhp_attr_uses, xhp_attr_uses_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_list self#on_hint env c.Aast.c_xhp_attr_uses
      in

      let (c_reqs, reqs_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_list (self#on_fst self#on_hint) env c.Aast.c_reqs
      in

      let (c_implements, implements_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_list self#on_hint env c.Aast.c_implements
      in

      let (c_where_constraints, where_constraints_err) =
        super#on_list
          self#on_where_constraint_hint
          env
          c.Aast.c_where_constraints
      in

      let (c_consts, consts_err) =
        super#on_list super#on_class_const env c.Aast.c_consts
      in

      let (c_typeconsts, typeconsts_err) =
        super#on_list super#on_class_typeconst_def env c.Aast.c_typeconsts
      in

      let (c_vars, vars_err) =
        super#on_list self#on_class_var env c.Aast.c_vars
      in

      let (c_enum, enum_err) =
        super#on_option super#on_enum_ env c.Aast.c_enum
      in

      let (c_methods, methods_err) =
        super#on_list self#on_method_ env c.Aast.c_methods
      in

      let (c_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env c.Aast.c_user_attributes
      in
      let (c_file_attributes, file_attributes_err) =
        super#on_list super#on_file_attribute env c.Aast.c_file_attributes
      in
      let err =
        self#plus_all
          [
            file_attributes_err;
            user_attributes_err;
            methods_err;
            enum_err;
            vars_err;
            typeconsts_err;
            consts_err;
            where_constraints_err;
            implements_err;
            reqs_err;
            xhp_attr_uses_err;
            xhp_attrs_err;
            uses_err;
            extends_err;
            tparams_err;
          ]
      in
      let c =
        Aast.
          {
            c with
            c_tparams;
            c_extends;
            c_uses;
            c_xhp_attr_uses;
            c_reqs;
            c_implements;
            c_where_constraints;
            c_consts;
            c_typeconsts;
            c_vars;
            c_enum;
            c_methods;
            c_xhp_attrs;
            c_user_attributes;
            c_file_attributes;
          }
      in
      (c, err)

    method! on_class_var env cv =
      let (cv_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env cv.Aast.cv_user_attributes
      in
      let (cv_expr, expr_err) =
        super#on_option super#on_expr env cv.Aast.cv_expr
      in
      let (cv_type, type_err) =
        let forbid_this =
          if cv.Aast.cv_is_static then
            not
            @@ Naming_attributes.mem SN.UserAttributes.uaLSB cv_user_attributes
          else
            false
        in
        let env = Env.create ~forbid_this () in
        super#on_type_hint env cv.Aast.cv_type
      in
      let cv = Aast.{ cv with cv_user_attributes; cv_type; cv_expr } in
      let err = self#plus_all [expr_err; type_err; user_attributes_err] in
      (cv, err)

    (* -- Functions --------------------------------------------------------- *)
    method! on_fun_ env f =
      let (f_ret, ret_err) =
        let env = Env.create ~forbid_this:false () in
        super#on_type_hint env f.Aast.f_ret
      in

      let (f_tparams, tparams_err) =
        super#on_list self#on_tparam env f.Aast.f_tparams
      in

      let (f_where_constraints, where_constraints_err) =
        super#on_list
          self#on_where_constraint_hint
          env
          f.Aast.f_where_constraints
      in

      let (f_params, params_err) =
        super#on_list self#on_fun_param env f.Aast.f_params
      in

      let (f_ctxs, ctxs_err) =
        super#on_option self#on_contexts env f.Aast.f_ctxs
      in

      let (f_unsafe_ctxs, unsafe_ctxs_err) =
        super#on_option self#on_contexts env f.Aast.f_unsafe_ctxs
      in

      let (f_body, body_err) = super#on_func_body env f.Aast.f_body in

      let (f_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env f.Aast.f_user_attributes
      in

      let err =
        self#plus_all
          [
            user_attributes_err;
            body_err;
            unsafe_ctxs_err;
            ctxs_err;
            params_err;
            where_constraints_err;
            tparams_err;
            ret_err;
          ]
      and f =
        Aast.
          {
            f with
            f_ret;
            f_tparams;
            f_where_constraints;
            f_params;
            f_ctxs;
            f_unsafe_ctxs;
            f_body;
            f_user_attributes;
          }
      in
      (f, err)

    (* -- Expressions ------------------------------------------------------- *)

    method! on_Cast _env hint expr =
      let env = Env.create ~forbid_this:false () in
      super#on_Cast env hint expr

    method! on_Is _env expr hint =
      let env = Env.create ~forbid_this:false () in
      super#on_Is env expr hint

    method! on_As _env expr hint is_final =
      let env = Env.create ~forbid_this:false () in
      super#on_As env expr hint is_final

    method! on_Upcast _env expr hint =
      let env = Env.create ~forbid_this:false () in
      super#on_Upcast env expr hint

    (* -- Hints   ----------------------------------------------------------- *)

    method! on_hint env hint =
      match hint with
      | (pos, Aast.Hthis) when env.Env.forbid_this ->
        let err = Err.naming @@ Naming_error.This_type_forbidden pos in
        ((pos, Aast.Herr), err)
      | _ -> super#on_hint env hint

    method! on_shape_field_info _env sfi =
      let env = Env.create ~forbid_this:false () in
      super#on_shape_field_info env sfi

    method! on_Hfun env hfun =
      let (hf_param_tys, param_tys_err) =
        super#on_list self#on_hint env hfun.Aast.hf_param_tys
      in
      let (hf_variadic_ty, variadic_ty_err) =
        super#on_option self#on_hint env hfun.Aast.hf_variadic_ty
      in
      let (hf_ctxs, contexts_err) =
        super#on_option self#on_contexts env hfun.Aast.hf_ctxs
      in
      let (hf_return_ty, return_ty_err) =
        let env = Env.create ~forbid_this:false () in
        self#on_hint env hfun.Aast.hf_return_ty
      in
      let err =
        self#plus_all
          [return_ty_err; contexts_err; variadic_ty_err; param_tys_err]
      in
      let hfun =
        Aast.{ hfun with hf_param_tys; hf_variadic_ty; hf_ctxs; hf_return_ty }
      in
      (Aast.Hfun hfun, err)

    method! on_targ _env targ =
      let env = Env.create ~forbid_this:false () in
      super#on_targ env targ

    (* -- Helpers ----------------------------------------------------------- *)
    method private on_fst f ctx (fst, snd) =
      let (fst, err) = f ctx fst in
      ((fst, snd), err)

    method private plus_all errs =
      List.fold_right ~init:self#zero ~f:self#plus errs
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
