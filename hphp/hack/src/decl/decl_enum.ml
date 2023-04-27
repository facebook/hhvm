(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module SN = Naming_special_names

type t = {
  base: Typing_defs.decl_ty;
      (** Underlying type of the enum, e.g. int or string.
          For subclasses of Enum, this is the type parameter of Enum.
          For enum classes, this is HH\MemberOf<E, I>. *)
  type_: Typing_defs.decl_ty;
      (** Type containing the enum name.
          For subclasses of Enum, this is also the type parameter of Enum. *)
  constraint_: Typing_defs.decl_ty option;
      (** Reflects what's after the [as] keyword in the enum definition. *)
  interface: Typing_defs.decl_ty option;
      (** For enum classes, this is the raw interface I, as provided by the user. *)
}

(** Figures out if a class needs to be treated like an enum. *)
let enum_kind name ~is_enum_class enum inner_ty ~get_ancestor =
  match enum with
  | None ->
    (match get_ancestor SN.FB.cEnum with
    | Some enum -> begin
      match get_node enum with
      | Tapply ((_, enum), [ty_exp]) when String.equal enum SN.FB.cEnum ->
        Some
          {
            base = ty_exp;
            type_ = ty_exp;
            constraint_ = None;
            interface = None;
          }
      | Tapply ((_, enum_class), _) when String.equal enum_class SN.FB.cEnum ->
        let ty_exp =
          (* The fallback if the class does not declare TInner (i.e. it is
           * abstract) is to use this::TInner
           *)
          match inner_ty with
          | None ->
            let this = Typing_defs_core.mk (get_reason enum, Tthis) in
            Typing_defs_core.mk
              (get_reason enum, Taccess (this, (get_pos enum, SN.FB.tInner)))
          | Some ty -> ty
        in
        Some
          {
            base = ty_exp;
            type_ = ty_exp;
            constraint_ = None;
            interface = None;
          }
      | _ -> None
    end
    | _ -> None)
  | Some enum ->
    let reason = get_reason enum.te_base in
    let pos = Reason.to_pos reason in
    let enum_type = Typing_defs.mk (reason, Tapply (name, [])) in
    let (te_base, te_interface) =
      if is_enum_class then
        let te_interface = enum.te_base in
        let te_base =
          Tapply ((pos, SN.Classes.cMemberOf), [enum_type; enum.te_base])
        in
        (* TODO(T77095784) make a new reason ! *)
        let te_base = mk (reason, te_base) in
        (te_base, Some te_interface)
      else
        (enum.te_base, None)
    in
    Some
      {
        base = te_base;
        type_ = Typing_defs.mk (get_reason enum.te_base, Tapply (name, []));
        constraint_ = enum.te_constraint;
        interface = te_interface;
      }

(** If a class is an Enum, we give all of the constants in the class the type
    of the Enum. We don't do this for Enum<mixed> and Enum<arraykey>, since
    that could *lose* type information. *)
let rewrite_class name ~is_enum_class enum inner_ty ~get_ancestor consts =
  match enum_kind name ~is_enum_class enum inner_ty ~get_ancestor with
  | None -> consts
  | Some { base = _; type_ = ty; constraint_ = _; interface = te_interface } ->
    let te_enum_class = Option.is_some te_interface in
    (match get_node ty with
    | Tmixed
    | Tprim Tarraykey ->
      consts
    | _ ->
      (* A special constant called "class" gets added, and we don't
       * want to rewrite its type.
       * Also for enum class, the type is set in the lowerer.
       *)
      SMap.mapi
        (fun k c ->
          if te_enum_class || String.equal k SN.Members.mClass then
            c
          else
            { c with cc_type = ty })
        consts)
