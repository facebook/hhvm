(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  type t = bool

  let empty = false
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_hint allow_retonly hint =
      match hint with
      | (pos, Aast.(Hprim Tvoid)) when not allow_retonly ->
        ( (pos, Aast.Herr),
          Err.naming @@ Naming_error.Return_only_typehint { pos; kind = `void }
        )
      | (pos, Aast.(Hprim Tnoreturn)) when not allow_retonly ->
        ( (pos, Aast.Herr),
          Err.naming
          @@ Naming_error.Return_only_typehint { pos; kind = `noreturn } )
      | (_, Aast.(Happly _ | Habstr _)) -> super#on_hint true hint
      | _ -> super#on_hint allow_retonly hint

    method! on_targ _ targ = super#on_targ true targ

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
        self#on_hint true hfun.Aast.hf_return_ty
      in
      let err =
        self#plus_all
          [return_ty_err; contexts_err; variadic_ty_err; param_tys_err]
      in
      let hfun =
        Aast.{ hfun with hf_param_tys; hf_variadic_ty; hf_ctxs; hf_return_ty }
      in
      (Aast.Hfun hfun, err)

    method! on_fun_ env f =
      let (f_ret, ret_err) = super#on_type_hint true f.Aast.f_ret in

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

    method! on_method_ env m =
      let (m_tparams, tparams_err) =
        super#on_list self#on_tparam env m.Aast.m_tparams
      in

      let (m_where_constraints, where_constraints_err) =
        super#on_list
          self#on_where_constraint_hint
          env
          m.Aast.m_where_constraints
      in

      let (m_params, params_err) =
        super#on_list self#on_fun_param env m.Aast.m_params
      in

      let (m_ctxs, ctxs_err) =
        super#on_option self#on_contexts env m.Aast.m_ctxs
      in

      let (m_unsafe_ctxs, unsafe_ctxs_err) =
        super#on_option self#on_contexts env m.Aast.m_unsafe_ctxs
      in

      let (m_body, body_err) = super#on_func_body env m.Aast.m_body in

      let (m_user_attributes, user_attributes_err) =
        super#on_list super#on_user_attribute env m.Aast.m_user_attributes
      in

      let (m_ret, ret_err) = super#on_type_hint true m.Aast.m_ret in

      let err =
        self#plus_all
          [
            ret_err;
            user_attributes_err;
            body_err;
            unsafe_ctxs_err;
            ctxs_err;
            params_err;
            where_constraints_err;
            tparams_err;
          ]
      and m =
        Aast.
          {
            m with
            m_tparams;
            m_where_constraints;
            m_params;
            m_ctxs;
            m_unsafe_ctxs;
            m_body;
            m_user_attributes;
            m_ret;
          }
      in
      (m, err)

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
