(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

(* This entire file is "private". Its sole consumer is Decl_provider.ml. *)

type class_t

val make_eager_class_decl : Decl_defs.decl_class_type -> class_t

val get :
  Provider_context.t ->
  string ->
  (Provider_context.t ->
  string ->
  Decl_defs.decl_class_type * Decl_store.class_members option) ->
  class_t option

module Api : sig
  (** This type "t" is what all APIs operate upon. It includes
  a "decl option". This provides context about how the specified
  class_t was fetched in the first place. It's used solely for telemetry,
  so that telemetry about APIs can be easily correlated with telemetry
  to the original call to the [get] which fetched the class_t in the
  first place.

  It also references the provider context that was used to query the class_t,
  as it might be needed when querying its members when the shallow decl where it
  was defines is evicted. For Zoncolan, this Provider_context.t is not
  available, as Zoncolan doesn't support eviction. *)
  type t =
    (Decl_counters.decl option[@opaque])
    * class_t
    * (Provider_context.t[@opaque]) option
  [@@deriving show]

  val need_init : t -> bool

  val abstract : t -> bool

  val final : t -> bool

  val has_const_attribute : t -> bool

  val deferred_init_members : t -> SSet.t

  val kind : t -> Ast_defs.classish_kind

  val allow_multiple_instantiations : t -> bool

  val is_xhp : t -> bool

  val name : t -> string

  val get_docs_url : t -> string option

  val get_module : t -> string option

  val internal : t -> bool

  val is_module_level_trait : t -> bool

  val pos : t -> Pos_or_decl.t

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

  val xhp_enum_values : t -> Ast_defs.xhp_enum_value list SMap.t

  val xhp_marked_empty : t -> bool

  val sealed_whitelist : t -> SSet.t option

  val decl_errors : t -> Decl_defs.decl_error list

  val get_ancestor : t -> string -> decl_ty option

  val has_ancestor : t -> string -> bool

  val requires_ancestor : t -> string -> bool

  val get_support_dynamic_type : t -> bool

  val all_ancestors : t -> (string * decl_ty) list

  val all_ancestor_names : t -> string list

  (** All the require extends and require implements requirements
    * These requirements impose a strict subtype constraint
    *)
  val all_ancestor_reqs : t -> requirement list

  val all_ancestor_req_names : t -> string list

  (** All the require class requirements
    * These requirements impose an equality constraint and are not
    * included in all_ancestor_reqs or all_ancestor_req_names
    *)
  val all_ancestor_req_class_requirements : t -> requirement list

  val get_const : t -> string -> class_const option

  val get_typeconst : t -> string -> typeconst_type option

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

  val props : t -> (string * class_elt) list

  val sprops : t -> (string * class_elt) list

  val methods : t -> (string * class_elt) list

  val smethods : t -> (string * class_elt) list

  (** Return the enforceability of the typeconst with the given name. A
      typeconst is enforceable if it was declared with the <<__Enforceable>>
      attribute, or if it overrides some ancestor typeconst with that attribute.
      Only enforceable typeconsts may be used in [is] or [as] expressions. The
      overriding behavior makes this expensive to compute in shallow decl, which
      is why this separate accessor is provided (rather than making this
      information available in the [ttc_enforceable] field, whose semantics
      differ between legacy and shallow decl due to the perf cost in shallow). *)
  val get_typeconst_enforceability :
    t -> string -> (Pos_or_decl.t * bool) option

  val valid_newable_class : t -> bool

  val overridden_method :
    t ->
    method_name:string ->
    is_static:bool ->
    get_class:(Provider_context.t -> string -> t option) ->
    class_elt option
end

val get_class_with_cache :
  Provider_context.t ->
  string ->
  Provider_backend.Decl_cache.t ->
  (Provider_context.t -> string -> Decl_defs.decl_class_type * 'a) ->
  class_t option
