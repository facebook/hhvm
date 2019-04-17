(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

module Classes : sig
  type key = StringKey.t
  type t

  val get : key -> t option
  val mem : key -> bool
  val find_unsafe : key -> t
end

type t = Classes.t

val need_init             : t -> bool
val members_fully_known   : t -> bool
val abstract              : t -> bool
val final                 : t -> bool
val const                 : t -> bool
val ppl                   : t -> bool

val deferred_init_members : t -> SSet.t
(** To be used only when {!ServerLocalConfig.shallow_class_decl} is not enabled.
    Raises [Failure] if used when shallow_class_decl is enabled. *)

val kind                  : t -> Ast.class_kind
val is_xhp                : t -> bool
val is_disposable         : t -> bool
val name                  : t -> string
val pos                   : t -> Pos.t
val tparams               : t -> decl tparam list
val construct             : t -> class_elt option * consistent_kind
val enum_type             : t -> enum_type option
val decl_errors           : t -> Errors.t option

val get_ancestor : t -> string -> decl ty option

val has_ancestor      : t -> string -> bool
val requires_ancestor : t -> string -> bool
val extends           : t -> string -> bool

val all_ancestors          : t -> (string * decl ty) Sequence.t
val all_ancestor_names     : t -> string Sequence.t
val all_ancestor_reqs      : t -> requirement Sequence.t
val all_ancestor_req_names : t -> string Sequence.t
val all_extends_ancestors  : t -> string Sequence.t

val get_const     : t -> string -> class_const option
val get_typeconst : t -> string -> typeconst_type option
val get_prop      : t -> string -> class_elt option
val get_sprop     : t -> string -> class_elt option
val get_method    : t -> string -> class_elt option
val get_smethod   : t -> string -> class_elt option

val has_const     : t -> string -> bool
val has_typeconst : t -> string -> bool
val has_prop      : t -> string -> bool
val has_sprop     : t -> string -> bool
val has_method    : t -> string -> bool
val has_smethod   : t -> string -> bool

val consts     : t -> (string * class_const) Sequence.t
val typeconsts : t -> (string * typeconst_type) Sequence.t
val props      : t -> (string * class_elt) Sequence.t
val sprops     : t -> (string * class_elt) Sequence.t
val methods    : t -> (string * class_elt) Sequence.t
val smethods   : t -> (string * class_elt) Sequence.t

(** The following functions return _all_ class member declarations defined in or
    inherited by this class with the given member name, including ones which
    were overridden, for purposes such as override checking. The list is ordered
    in reverse with respect to the linearization (so members defined in more
    derived classes occur later in the list).

    To be used only when {!ServerLocalConfig.shallow_class_decl} is enabled.
    Raises [Failure] if used when shallow_class_decl is not enabled. *)
val all_inherited_methods    : t -> string -> class_elt list
val all_inherited_smethods   : t -> string -> class_elt list

val shallow_decl : t -> Shallow_decl_defs.shallow_class
(** Return the shallow declaration for the given class.

    To be used only when {!ServerLocalConfig.shallow_class_decl} is enabled.
    Raises [Failure] if used when shallow_class_decl is not enabled. *)
