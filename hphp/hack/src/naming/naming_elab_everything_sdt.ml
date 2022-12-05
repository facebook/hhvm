(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env : sig
  type t

  val empty : t

  val in_is_as : t -> bool

  val in_enum_class : t -> bool

  val set_in_is_as : t -> in_is_as:bool -> t

  val set_in_enum_class : t -> in_enum_class:bool -> t
end = struct
  type t = {
    in_is_as: bool;
    in_enum_class: bool;
  }

  let empty = { in_is_as = false; in_enum_class = false }

  let in_is_as { in_is_as; _ } = in_is_as

  let set_in_is_as t ~in_is_as = { t with in_is_as }

  let in_enum_class { in_enum_class; _ } = in_enum_class

  let set_in_enum_class t ~in_enum_class = { t with in_enum_class }
end

let wrap_supportdyn p h = Aast.Happly ((p, SN.Classes.cSupportDyn), [(p, h)])

let on_expr_ (env, expr_, err) =
  let env =
    match expr_ with
    | Aast.(Is _ | As _) -> Env.set_in_is_as env ~in_is_as:true
    | _ -> env
  in
  Naming_phase_pass.Cont.next (env, expr_, err)

(* Before the refactor, `everything_sdt` was coupled with elaboration of
   enum classes so we applied `Hlike` to the enum class bound hint at the
   point of the elaboration and we would pick up the hint generated in
   the overriden `on_enum_` method, below.
   Since this visitor is applied _after_ enum class elaboration, we need to
   select it from the list of types in c_extends and to apply.
   This is only necessary for concrete enum classes and the form of the
   hint is very predictable
*)
let apply_like_to_enum_bound hint =
  match hint with
  | ( enum_class_pos,
      Aast.Happly
        ( enum_class_nm,
          [(member_of_pos, Aast.Happly (member_of_nm, [enum_hint; enum_base]))]
        ) )
    when String.(
           equal (snd enum_class_nm) SN.Classes.cHH_BuiltinEnumClass
           && equal (snd member_of_nm) SN.Classes.cMemberOf) ->
    let enum_base = (fst enum_base, Aast.Hlike enum_base) in
    ( enum_class_pos,
      Aast.(
        Happly
          ( enum_class_nm,
            [(member_of_pos, Happly (member_of_nm, [enum_hint; enum_base]))] ))
    )
  | _ -> hint

let on_hint (env, hint, err) =
  let hint =
    match hint with
    | (pos, (Aast.(Hmixed | Hnonnull) as hint_)) when not @@ Env.in_is_as env ->
      (pos, wrap_supportdyn pos hint_)
    | (pos, (Aast.Hshape Aast.{ nsi_allows_unknown_fields = true; _ } as hint_))
      ->
      (pos, wrap_supportdyn pos hint_)
    | (pos, Aast.Hfun hint_fun) ->
      let ((hf_return_pos, _) as hf_return_ty) = hint_fun.Aast.hf_return_ty in
      let hf_return_ty = (hf_return_pos, Aast.Hlike hf_return_ty) in
      let hint_ = Aast.(Hfun { hint_fun with hf_return_ty }) in
      (pos, wrap_supportdyn pos hint_)
    | _ -> hint
  in
  Naming_phase_pass.Cont.next (env, hint, err)

let on_fun_ (env, f, err) =
  let (pos, _) = f.Aast.f_name in
  let f_user_attributes =
    Aast.
      {
        ua_name = (pos, SN.UserAttributes.uaSupportDynamicType);
        ua_params = [];
      }
    :: f.Aast.f_user_attributes
  in
  Naming_phase_pass.Cont.next (env, Aast.{ f with f_user_attributes }, err)

let on_tparam (env, t, err) =
  let (pos, _) = t.Aast.tp_name in
  let tp_constraints =
    (Ast_defs.Constraint_as, (pos, wrap_supportdyn pos Aast.Hmixed))
    :: t.Aast.tp_constraints
  in
  Naming_phase_pass.Cont.next (env, Aast.{ t with tp_constraints }, err)

let on_class_top_down (env, c, err) =
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | _ -> false
  in
  let env = Env.set_in_enum_class env ~in_enum_class in
  Naming_phase_pass.Cont.next (env, c, err)

let on_class_ (env, c, err) =
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
    if Env.in_enum_class env then
      List.map ~f:apply_like_to_enum_bound c.Aast.c_extends
    else
      c.Aast.c_extends
  in
  Naming_phase_pass.Cont.next
    (env, Aast.{ c with c_user_attributes; c_extends }, err)

let on_enum_ (env, e, err) =
  let e_base =
    if Env.in_enum_class env then
      let ((pos, _) as e_base) = e.Aast.e_base in
      (pos, Aast.Hlike e_base)
    else
      e.Aast.e_base
  in
  Naming_phase_pass.Cont.next (env, Aast.{ e with e_base }, err)

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

let visitor = Naming_phase_pass.mk_visitor [top_down_pass; bottom_up_pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
