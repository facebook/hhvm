(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type enf =
  (* The type is fully enforced *)
  | Enforced of Typing_defs.decl_ty
  (* The type is not fully enforced, but is enforced at the given ty, if present *)
  | Unenforced of Typing_defs.decl_ty option * Typing_reason.pessimise_reason

type 'a class_or_typedef_result =
  | ClassResult of 'a
  | TypedefResult of Typing_defs.typedef_type

type fun_kind =
  | Function
  | Abstract_method
  | Concrete_method

module type Provider = sig
  (** [t] is the type of the context that classes and typedefs can be found in *)
  type t

  (** [class_t] is the type that represents a class *)
  type class_t

  val get_tcopt : t -> TypecheckerOptions.t

  val get_class_or_typedef :
    t -> string -> class_t class_or_typedef_result option

  val get_class : t -> string -> class_t option
end

module type ContextAccess = sig
  include Provider

  (** [get_typeconst ctx cls name] gets the definition of the type constant
      [name] from [cls] or ancestor if it exists. *)
  val get_typeconst_type : t -> class_t -> string -> Typing_defs.decl_ty option

  val get_tparams : class_t -> Typing_defs.decl_tparam list

  val is_final : class_t -> bool

  val get_name : class_t -> string

  (** [get_enum_type cls] returns the enumeration type if [cls] is an enum. *)
  val get_enum_type : class_t -> Typing_defs.enum_type option
end

module type ShallowProvider =
  Provider with type class_t := Shallow_decl_defs.shallow_class

module ShallowContextAccess : functor (CA : ShallowProvider) ->
  ContextAccess
    with type class_t = Shallow_decl_defs.shallow_class
     and type t = CA.t

module Enforce : functor (ContextAccess : ContextAccess) -> sig
  val get_enforcement :
    return_from_async:bool ->
    this_class:ContextAccess.class_t option ->
    ContextAccess.t ->
    Typing_defs.decl_ty ->
    enf
end

module Pessimize : functor (Provider : ShallowProvider) -> sig
  (** Pessimise the type of a property if in implicit pessimisation mode and no_auto_likes is false *)
  val pessimise_prop_type :
    is_xhp_attr:bool ->
    this_class:Shallow_decl_defs.shallow_class option ->
    no_auto_likes:bool ->
    Provider.t ->
    Pos_or_decl.t ->
    Typing_defs.decl_ty ->
    Typing_defs.decl_ty

  (** If the return type is not enforceable, turn it into a like type (~ty) otherwise
    return the original function type. Also add supportdyn<mixed> to the type parameters. *)
  val pessimise_fun_type :
    fun_kind:fun_kind ->
    this_class:Shallow_decl_defs.shallow_class option ->
    no_auto_likes:bool ->
    Provider.t ->
    Pos_or_decl.t ->
    Typing_defs.decl_ty ->
    Typing_defs.decl_ty

  val implicit_sdt_for_class :
    Provider.t -> Shallow_decl_defs.shallow_class option -> bool
end

(** Add as supportdyn<mixed> constraints to the type parameters *)
val add_supportdyn_constraints :
  Pos_or_decl.t ->
  Typing_defs.decl_ty Typing_defs_core.tparam list ->
  Typing_defs.decl_ty Typing_defs_core.tparam list

(** Add as supportdyn<mixed> constraints to the type parameters if in implicit pessimisation mode.*)
val maybe_add_supportdyn_constraints :
  this_class:Shallow_decl_defs.shallow_class option ->
  Provider_context.t ->
  Pos_or_decl.t ->
  Typing_defs.decl_ty Typing_defs_core.tparam list ->
  Typing_defs.decl_ty Typing_defs_core.tparam list

val supportdyn_mixed :
  Pos_or_decl.t -> Typing_defs.Reason.decl_t -> Typing_defs.decl_ty
