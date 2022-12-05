(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  let in_is_as
      Naming_phase_env.
        { elab_everything_sdt = Elab_everything_sdt.{ in_is_as; _ }; _ } =
    in_is_as

  let set_in_is_as t ~in_is_as =
    Naming_phase_env.
      {
        t with
        elab_everything_sdt =
          Elab_everything_sdt.{ t.elab_everything_sdt with in_is_as };
      }

  let in_enum_class
      Naming_phase_env.
        { elab_everything_sdt = Elab_everything_sdt.{ in_enum_class; _ }; _ } =
    in_enum_class

  let set_in_enum_class t ~in_enum_class =
    Naming_phase_env.
      {
        t with
        elab_everything_sdt =
          Elab_everything_sdt.{ t.elab_everything_sdt with in_enum_class };
      }

  let everything_sdt Naming_phase_env.{ everything_sdt; _ } = everything_sdt
end

let wrap_supportdyn p h = Aast.Happly ((p, SN.Classes.cSupportDyn), [(p, h)])

let on_expr_ (env, expr_, err) =
  let env =
    match expr_ with
    | Aast.(Is _ | As _) -> Env.set_in_is_as env ~in_is_as:true
    | _ -> env
  in
  Ok (env, expr_, err)

let on_hint (env, hint, err) =
  let hint =
    if Env.everything_sdt env then
      match hint with
      | (pos, (Aast.(Hmixed | Hnonnull) as hint_)) when not @@ Env.in_is_as env
        ->
        (pos, wrap_supportdyn pos hint_)
      | ( pos,
          (Aast.Hshape Aast.{ nsi_allows_unknown_fields = true; _ } as hint_) )
        ->
        (pos, wrap_supportdyn pos hint_)
      | (pos, Aast.Hfun hint_fun) ->
        let ((hf_return_pos, _) as hf_return_ty) = hint_fun.Aast.hf_return_ty in
        let hf_return_ty = (hf_return_pos, Aast.Hlike hf_return_ty) in
        let hint_ = Aast.(Hfun { hint_fun with hf_return_ty }) in
        (pos, wrap_supportdyn pos hint_)
      | _ -> hint
    else
      hint
  in
  Ok (env, hint, err)

let on_fun_ (env, f, err) =
  let f =
    if Env.everything_sdt env then
      let (pos, _) = f.Aast.f_name in
      let f_user_attributes =
        Aast.
          {
            ua_name = (pos, SN.UserAttributes.uaSupportDynamicType);
            ua_params = [];
          }
        :: f.Aast.f_user_attributes
      in
      Aast.{ f with f_user_attributes }
    else
      f
  in
  Ok (env, f, err)

let on_tparam (env, t, err) =
  let t =
    if Env.everything_sdt env then
      let (pos, _) = t.Aast.tp_name in
      let tp_constraints =
        (Ast_defs.Constraint_as, (pos, wrap_supportdyn pos Aast.Hmixed))
        :: t.Aast.tp_constraints
      in
      Aast.{ t with tp_constraints }
    else
      t
  in
  Ok (env, t, err)

let on_class_top_down (env, c, err) =
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | _ -> false
  in
  let env = Env.set_in_enum_class env ~in_enum_class in
  Ok (env, c, err)

let on_class_ (env, c, err) =
  let c =
    if Env.everything_sdt env then
      let (pos, _) = c.Aast.c_name in
      let c_user_attributes =
        match c.Aast.c_kind with
        | Ast_defs.(Cclass _ | Cinterface | Ctrait) ->
          Aast.
            {
              ua_name = (pos, SN.UserAttributes.uaSupportDynamicType);
              ua_params = [];
            }
          :: c.Aast.c_user_attributes
        | _ -> c.Aast.c_user_attributes
      in
      Aast.{ c with c_user_attributes }
    else
      c
  in
  Ok (env, c, err)

let on_enum_ (env, e, err) =
  let e =
    if Env.everything_sdt env then
      let e_base =
        if Env.in_enum_class env then
          let ((pos, _) as e_base) = e.Aast.e_base in
          (pos, Aast.Hlike e_base)
        else
          e.Aast.e_base
      in
      Aast.{ e with e_base }
    else
      e
  in
  Ok (env, e, err)

let top_down_pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_class_ = Some on_class_top_down;
        on_expr_ = Some on_expr_;
      })

let bottom_up_pass =
  Naming_phase_pass.(
    bottom_up
      {
        identity with
        on_hint = Some on_hint;
        on_fun_ = Some on_fun_;
        on_tparam = Some on_tparam;
        on_class_ = Some on_class_;
        on_enum_ = Some on_enum_;
      })
