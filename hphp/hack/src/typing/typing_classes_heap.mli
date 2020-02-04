(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

module Classes : sig
  type key = StringKey.t

  type t

  val get : Provider_context.t -> key -> t option

  val mem : Provider_context.t -> key -> bool

  val find_unsafe : Provider_context.t -> key -> t
end

module Api : sig
  type t = Classes.t

  val need_init : t -> bool

  val members_fully_known : t -> bool

  val abstract : t -> bool

  val final : t -> bool

  val const : t -> bool

  val ppl : t -> bool

  (** To be used only when {!ServerLocalConfig.shallow_class_decl} is not enabled.
      Raises [Failure] if used when shallow_class_decl is enabled. *)
  val deferred_init_members : t -> SSet.t

  val kind : t -> Ast_defs.class_kind

  val is_xhp : t -> bool

  val is_disposable : t -> bool

  val name : t -> string

  val pos : t -> Pos.t

  val tparams : t -> decl_tparam list

  val where_constraints : t -> decl_where_constraint list

  val all_where_constraints_on_this : t -> decl_where_constraint list

  val upper_bounds_on_this : t -> decl_ty list

  val upper_bounds_on_this_from_constraints : t -> decl_ty list

  val has_upper_bounds_on_this_from_constraints : t -> bool

  val lower_bounds_on_this : t -> decl_ty list

  val lower_bounds_on_this_from_constraints : t -> decl_ty list

  val has_lower_bounds_on_this_from_constraints : t -> bool

  val construct : t -> class_elt option * consistent_kind

  val enum_type : t -> enum_type option

  val sealed_whitelist : t -> SSet.t option

  val decl_errors : t -> Errors.t option

  val get_ancestor : t -> string -> decl_ty option

  val has_ancestor : t -> string -> bool

  val requires_ancestor : t -> string -> bool

  val extends : t -> string -> bool

  val all_ancestors : t -> (string * decl_ty) list

  val all_ancestor_names : t -> string list

  val all_ancestor_reqs : t -> requirement list

  val all_ancestor_req_names : t -> string Sequence.t

  val all_extends_ancestors : t -> string Sequence.t

  val get_const : t -> string -> class_const option

  val get_typeconst : t -> string -> typeconst_type option

  val get_pu_enum : t -> string -> pu_enum_type option

  val get_prop : t -> string -> class_elt option

  val get_sprop : t -> string -> class_elt option

  val get_method : t -> string -> class_elt option

  val get_smethod : t -> string -> class_elt option

  val get_any_method : is_static:bool -> t -> string -> class_elt option

  val has_const : t -> string -> bool

  val has_typeconst : t -> string -> bool

  val has_prop : t -> string -> bool

  val has_sprop : t -> string -> bool

  val has_method : t -> string -> bool

  val has_smethod : t -> string -> bool

  val consts : t -> (string * class_const) list

  val typeconsts : t -> (string * typeconst_type) list

  val pu_enums : t -> (string * pu_enum_type) list

  val props : t -> (string * class_elt) list

  val sprops : t -> (string * class_elt) list

  val methods : t -> (string * class_elt) list

  val smethods : t -> (string * class_elt) list

  (** The following functions return _all_ class member declarations defined in or
      inherited by this class with the given member name, including ones which
      were overridden, for purposes such as override checking. The list is ordered
      in reverse with respect to the linearization (so members defined in more
      derived classes occur later in the list).

      To be used only when {!ServerLocalConfig.shallow_class_decl} is enabled.
      Raises [Failure] if used when shallow_class_decl is not enabled. *)
  val all_inherited_methods : t -> string -> class_elt list

  val all_inherited_smethods : t -> string -> class_elt list

  (** Return the shallow declaration for the given class.

      To be used only when {!ServerLocalConfig.shallow_class_decl} is enabled.
      Raises [Failure] if used when shallow_class_decl is not enabled. *)
  val shallow_decl : t -> Shallow_decl_defs.shallow_class
end

(** Implementation detail, do not use. For use in [Decl_provider] only. *)
val compute_class_decl_no_cache :
  Provider_context.t -> string -> Classes.t option
