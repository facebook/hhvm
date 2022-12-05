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
  type t = {
    in_is_as: bool;
    in_enum_class: bool;
  }

  let empty = { in_is_as = false; in_enum_class = false }

  let create ?(in_is_as = false) ?(in_enum_class = false) () =
    { in_is_as; in_enum_class }
end

let wrap_supportdyn p h = Aast.Happly ((p, SN.Classes.cSupportDyn), [(p, h)])

let visitor =
  object (self)
    inherit [_] Aast_defs.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_Is env expr hint =
      super#on_Is Env.{ env with in_is_as = true } expr hint

    method! on_As env expr hint is_final =
      super#on_As Env.{ env with in_is_as = true } expr hint is_final

    method! on_hint (Env.{ in_is_as; _ } as env) hint =
      let hint = super#on_hint env hint in
      match hint with
      | (pos, (Aast.(Hmixed | Hnonnull) as hint_)) when not in_is_as ->
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

    method! on_fun_ env f =
      let f = super#on_fun_ env f in
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

    method! on_tparam env t =
      let t = super#on_tparam env t in
      let (pos, _) = t.Aast.tp_name in
      let tp_constraints =
        (Ast_defs.Constraint_as, (pos, wrap_supportdyn pos Aast.Hmixed))
        :: t.Aast.tp_constraints
      in
      Aast.{ t with tp_constraints }

    method! on_class_ env c =
      let in_enum_class =
        match c.Aast.c_kind with
        | Ast_defs.Cenum_class _ -> true
        | _ -> false
      in
      let env = Env.{ env with in_enum_class } in
      let c = super#on_class_ env c in
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
      let c_extends =
        if in_enum_class then
          List.map ~f:self#apply_like_to_enum_bound c.Aast.c_extends
        else
          c.Aast.c_extends
      in
      Aast.{ c with c_user_attributes; c_extends }

    (* Before the refactor, `everything_sdt` was coupled with elaboration of
       enum classes so we applied `Hlike` to the enum class bound hint at the
       point of the elaboration and we would pick up the hint generated in
       the overriden `on_enum_` method, below.
       Since this visitor is applied _after_ enum class elaboration, we need to
       select it from the list of types in c_extends and to apply.
       This is only necessary for concrete enum classes and the form of the
       hint is very predictable
    *)
    method private apply_like_to_enum_bound hint =
      match hint with
      | ( enum_class_pos,
          Aast.Happly
            ( enum_class_nm,
              [
                ( member_of_pos,
                  Aast.Happly (member_of_nm, [enum_hint; enum_base]) );
              ] ) )
        when String.(
               equal (snd enum_class_nm) SN.Classes.cHH_BuiltinEnumClass
               && equal (snd member_of_nm) SN.Classes.cMemberOf) ->
        let enum_base = (fst enum_base, Aast.Hlike enum_base) in
        ( enum_class_pos,
          Aast.(
            Happly
              ( enum_class_nm,
                [(member_of_pos, Happly (member_of_nm, [enum_hint; enum_base]))]
              )) )
      | _ -> hint

    method! on_enum_ (Env.{ in_enum_class; _ } as env) e =
      let e_base =
        if in_enum_class then
          let ((pos, _) as e_base) = e.Aast.e_base in
          (pos, Aast.Hlike e_base)
        else
          e.Aast.e_base
      in
      super#on_enum_ env Aast.{ e with e_base }
  end

let elab f ?(env = Env.empty) elem = f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
