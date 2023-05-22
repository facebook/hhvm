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

  let set_under_no_auto_dynamic t ~under_no_auto_dynamic =
    Naming_phase_env.
      {
        t with
        elab_everything_sdt =
          Elab_everything_sdt.
            { t.elab_everything_sdt with under_no_auto_dynamic };
      }

  let everything_sdt Naming_phase_env.{ everything_sdt; _ } = everything_sdt

  let under_no_auto_dynamic
      Naming_phase_env.
        {
          elab_everything_sdt = Elab_everything_sdt.{ under_no_auto_dynamic; _ };
          _;
        } =
    under_no_auto_dynamic

  let implicit_sdt env = everything_sdt env && not (under_no_auto_dynamic env)
end

let no_auto_dynamic_attr ua =
  Naming_attributes.mem SN.UserAttributes.uaNoAutoDynamic ua

let wrap_supportdyn p h = Aast.Happly ((p, SN.Classes.cSupportDyn), [(p, h)])

let wrap_like ((pos, _) as hint) = (pos, Aast.Hlike hint)

let on_expr_ expr_ ~ctx =
  let ctx =
    match expr_ with
    | Aast.(Is _ | As _) -> Env.set_in_is_as ctx ~in_is_as:true
    | _ -> ctx
  in
  (ctx, Ok expr_)

let on_hint hint ~ctx =
  let hint =
    if Env.implicit_sdt ctx then
      match hint with
      | (pos, (Aast.(Hmixed | Hnonnull) as hint_)) when not @@ Env.in_is_as ctx
        ->
        (pos, wrap_supportdyn pos hint_)
      | ( pos,
          (Aast.Hshape Aast.{ nsi_allows_unknown_fields = true; _ } as hint_) )
        ->
        (pos, wrap_supportdyn pos hint_)
      (* Return types and inout parameter types are pessimised *)
      | (pos, Aast.Hfun hint_fun) ->
        let hf_param_tys =
          match
            List.map2
              hint_fun.Aast.hf_param_info
              hint_fun.Aast.hf_param_tys
              ~f:(fun p ty ->
                match p with
                | Some { Aast.hfparam_kind = Ast_defs.Pinout _; _ } ->
                  wrap_like ty
                | _ -> ty)
          with
          | List.Or_unequal_lengths.Ok res -> res
          (* Shouldn't happen *)
          | List.Or_unequal_lengths.Unequal_lengths ->
            hint_fun.Aast.hf_param_tys
        in
        let hf_return_ty = wrap_like hint_fun.Aast.hf_return_ty in
        let hint_ = Aast.(Hfun { hint_fun with hf_return_ty; hf_param_tys }) in
        (pos, wrap_supportdyn pos hint_)
      | _ -> hint
    else
      hint
  in
  (ctx, Ok hint)

let on_fun_def_top_down fd ~ctx =
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:
        (no_auto_dynamic_attr Aast.(fd.fd_fun.f_user_attributes))
  in
  (ctx, Ok fd)

let on_fun_def fd ~ctx =
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:
        (no_auto_dynamic_attr Aast.(fd.fd_fun.f_user_attributes))
  in
  let fd_fun = fd.Aast.fd_fun in
  let fd_fun =
    if Env.implicit_sdt ctx then
      let (pos, _) = fd.Aast.fd_name in
      let f_user_attributes =
        Aast.
          {
            ua_name = (pos, SN.UserAttributes.uaSupportDynamicType);
            ua_params = [];
          }
        :: fd_fun.Aast.f_user_attributes
      in
      Aast.{ fd_fun with f_user_attributes }
    else
      fd_fun
  in
  let fd = Aast.{ fd with fd_fun } in
  (ctx, Ok fd)

let on_tparam t ~ctx =
  let t =
    if Env.implicit_sdt ctx then
      let (pos, _) = t.Aast.tp_name in
      let tp_constraints =
        (Ast_defs.Constraint_as, (pos, wrap_supportdyn pos Aast.Hmixed))
        :: t.Aast.tp_constraints
      in
      Aast.{ t with tp_constraints }
    else
      t
  in
  (ctx, Ok t)

let on_typedef_top_down t ~ctx =
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:(no_auto_dynamic_attr t.Aast.t_user_attributes)
  in
  (ctx, Ok t)

let on_typedef t ~ctx =
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:(no_auto_dynamic_attr t.Aast.t_user_attributes)
  in
  Aast_defs.(
    let (pos, _) = t.Aast.t_name in
    let t_as_constraint =
      if Env.implicit_sdt ctx then
        match t.t_as_constraint with
        | Some _ -> t.t_as_constraint
        | None -> begin
          (* If this isn't just a type alias then we need to add supportdyn<mixed> as upper bound *)
          match t.t_vis with
          | Transparent -> None
          | _ -> Some (pos, wrap_supportdyn pos Aast.Hmixed)
        end
      else
        t.t_as_constraint
    in
    (ctx, Ok Aast.{ t with t_as_constraint }))

let on_class_top_down c ~ctx =
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | _ -> false
  in
  let ctx = Env.set_in_enum_class ctx ~in_enum_class in
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:(no_auto_dynamic_attr c.Aast.c_user_attributes)
  in
  (ctx, Ok c)

let on_class_ c ~ctx =
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:(no_auto_dynamic_attr c.Aast.c_user_attributes)
  in
  let c =
    if Env.implicit_sdt ctx then
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
  (ctx, Ok c)

let on_class_c_consts c_consts ~ctx =
  let c_consts =
    if Env.everything_sdt ctx && Env.in_enum_class ctx then
      let elab_hint = function
        | ( pos,
            Aast.(
              Happly ((p_member_of, c_member_of), [((p1, Happly _) as h1); h2]))
          )
          when String.equal c_member_of SN.Classes.cMemberOf ->
          (pos, Aast.(Happly ((p_member_of, c_member_of), [h1; (p1, Hlike h2)])))
        | hint -> hint
      in
      List.map
        ~f:(fun c_const ->
          Aast.
            { c_const with cc_type = Option.map ~f:elab_hint c_const.cc_type })
        c_consts
    else
      c_consts
  in
  (ctx, Ok c_consts)

let on_enum_ e ~ctx =
  let e =
    if Env.everything_sdt ctx && Env.in_enum_class ctx then
      let e_base = wrap_like e.Aast.e_base in
      Aast.{ e with e_base }
    else
      e
  in
  (ctx, Ok e)

let on_method_top_down m ~ctx =
  let ctx =
    Env.set_under_no_auto_dynamic
      ctx
      ~under_no_auto_dynamic:
        (no_auto_dynamic_attr m.Aast.m_user_attributes
        || Env.under_no_auto_dynamic ctx)
  in
  (ctx, Ok m)

let top_down_pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.(
    top_down
      Aast.Pass.
        {
          id with
          on_ty_fun_def = Some on_fun_def_top_down;
          on_ty_class_ = Some on_class_top_down;
          on_ty_method_ = Some on_method_top_down;
          on_ty_expr_ = Some on_expr_;
          on_ty_typedef = Some on_typedef_top_down;
        })

let bottom_up_pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_hint = Some on_hint;
        on_ty_fun_def = Some on_fun_def;
        on_ty_tparam = Some on_tparam;
        on_ty_class_ = Some on_class_;
        on_fld_class__c_consts = Some on_class_c_consts;
        on_ty_enum_ = Some on_enum_;
        on_ty_typedef = Some on_typedef;
      }
