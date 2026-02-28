(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
val enum_kind :
  Typing_defs.pos_id ->
  is_enum_class:bool ->
  add_like:bool ->
  Typing_defs.enum_type option ->
  Typing_defs.decl_ty option ->
  get_ancestor:(string -> Typing_defs.decl_phase Typing_defs.ty option) ->
  t option

(** If a class is an Enum, we give all of the constants in the class the type
    of the Enum. We don't do this for Enum<mixed> and Enum<arraykey>, since
    that could *lose* type information. *)
val rewrite_class :
  Typing_defs.pos_id ->
  is_enum_class:bool ->
  Typing_defs.enum_type option ->
  Typing_defs.decl_ty option ->
  get_ancestor:(string -> Typing_defs.decl_phase Typing_defs.ty option) ->
  Typing_defs.class_const SMap.t ->
  Typing_defs.class_const SMap.t
