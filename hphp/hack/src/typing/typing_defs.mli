(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* "include" typing_defs_core.mli *)
include module type of struct
  include Typing_defs_core
end

type class_const_from =
  | Self
  | From of string
[@@deriving eq, show]

type class_const_ref = class_const_from * string [@@deriving eq, show]

module CCR : sig
  type t = class_const_ref [@@deriving show]

  val compare_class_const_from : class_const_from -> class_const_from -> int

  val compare : class_const_from * string -> class_const_from * string -> int
end

module CCRSet : sig
  include module type of struct
    include Stdlib.Set.Make (CCR)
  end

  type t_as_list = CCR.t list [@@deriving show]

  val pp :
    Ppx_deriving_runtime.Format.formatter -> t -> Ppx_deriving_runtime.unit

  val show : t -> Ppx_deriving_runtime.string
end

type const_decl = {
  cd_pos: Pos_or_decl.t;
  cd_type: decl_ty;
  cd_value: string option;
  cd_package: Aast_defs.package_membership option;
}
[@@deriving eq, show]

type package_requirement =
  | RPRequire of pos_string
  | RPSoft of pos_string
  | RPNormal
[@@deriving eq, show]

type class_elt = {
  ce_visibility: ce_visibility;
  ce_type: decl_ty Hh_prelude.Lazy.t;
  ce_origin: string;  (** identifies the class from which this elt originates *)
  ce_deprecated: string option;
  ce_pos: Pos_or_decl.t Hh_prelude.Lazy.t;  (** pos of the type of the elt *)
  ce_flags: Typing_defs_flags.ClassElt.t;
  ce_sealed_allowlist: SSet.t option;
  ce_sort_text: string option;
  ce_overlapping_tparams: SSet.t option;
  ce_package_requirement: package_requirement option;
}
[@@deriving show]

type fun_elt = {
  fe_deprecated: string option;
  fe_module: Ast_defs.id option;
  fe_package: Aast_defs.package_membership option;
  fe_internal: bool;  (** Top-level functions have limited visibilities *)
  fe_type: decl_ty;
  fe_pos: Pos_or_decl.t;
  fe_php_std_lib: bool;
  fe_support_dynamic_type: bool;
  fe_no_auto_dynamic: bool;
  fe_no_auto_likes: bool;
  fe_package_requirement: package_requirement;
}
[@@deriving eq, show]

type class_const_kind =
  | CCAbstract of bool (* has default *)
  | CCConcrete
[@@deriving eq, show]

type class_const = {
  cc_synthesized: bool;
  cc_abstract: class_const_kind;
  cc_pos: Pos_or_decl.t;
  cc_type: decl_ty;
  cc_origin: string;
      (** identifies the class from which this const originates *)
  cc_refs: class_const_ref list;
      (** references to the constants used in the initializer *)
}
[@@deriving show]

type module_def_type = { mdt_pos: Pos_or_decl.t } [@@deriving show]

type requirement = Pos_or_decl.t * decl_ty [@@deriving show]

type constraint_requirement =
  | CR_Equal of requirement
  | CR_Subtype of requirement
[@@deriving eq, show]

val to_requirement : constraint_requirement -> requirement

type abstract_typeconst = {
  atc_as_constraint: decl_ty option;
  atc_super_constraint: decl_ty option;
  atc_default: decl_ty option;
}
[@@deriving show]

type concrete_typeconst = { tc_type: decl_ty } [@@deriving show]

type partially_abstract_typeconst = {
  patc_constraint: decl_ty;
  patc_type: decl_ty;
}
[@@deriving show]

type typeconst =
  | TCAbstract of abstract_typeconst
  | TCConcrete of concrete_typeconst
[@@deriving eq, show]

type typeconst_type = {
  ttc_synthesized: bool;
  ttc_name: pos_id;
  ttc_kind: typeconst;
  ttc_origin: string;
  ttc_enforceable: Pos_or_decl.t * bool;
  ttc_reifiable: Pos_or_decl.t option;
  ttc_concretized: bool;
  ttc_is_ctx: bool;
}
[@@deriving show]

type enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
  te_includes: decl_ty list;
}
[@@deriving eq, show]

type typedef_case_type_variant = decl_ty * decl_where_constraint list
[@@deriving eq, show]

type typedef_type_assignment =
  | SimpleTypeDef of Ast_defs.typedef_visibility * decl_ty
  | CaseType of typedef_case_type_variant * typedef_case_type_variant list
[@@deriving eq, show]

type typedef_type = {
  td_module: Ast_defs.id option;
  td_pos: Pos_or_decl.t;
  td_tparams: decl_tparam list;
  td_as_constraint: decl_ty option;
  td_super_constraint: decl_ty option;
  td_type_assignment: typedef_type_assignment;
  td_is_ctx: bool;
  td_attributes: user_attribute list;
  td_internal: bool;
  td_docs_url: string option;
  td_package: Aast_defs.package_membership option;
}
[@@deriving eq, show]

type phase_ty =
  | DeclTy of decl_ty
  | LoclTy of locl_ty

type deserialization_error =
  | Wrong_phase of string
  | Not_supported of string
      (** The specific type or some component thereof is not one that we support
          deserializing, usually because not enough information was serialized to be
          able to deserialize it again. *)
  | Deserialization_error of string
      (** The input JSON was invalid for some reason. *)
[@@deriving show]

(** How should we treat the wildcard character _ when localizing?
 *  1. Generate a fresh type variable, e.g. in type argument to constructor or function,
 *     or in a lambda parameter or return type.
 *       Example: foo<shape('a' => _)>($myshape);
 *       Example: ($v : vec<_>) ==> $v[0]
 *  2. As a placeholder in a formal higher-kinded type parameter
 *       Example: function test<T1, T2<_>>() // T2 is HK and takes a type to a type
 *  3. Generate a fresh generic (aka Skolem variable), e.g. in `is` or `as` test
 *       Example: if ($x is Vector<_>) { // $x has type Vector<T#1> }
 *  4. Reject, when in a type argument to a generic parameter marked <<__Explicit>>
 *       Example: makeVec<_>(3)  where function makeVec<<<__Explicit>> T>(T $_): void
 *  5. Reject, because the type must be explicit.
 *  6. (Specially for case type checking). Replace any type argument by a fresh generic.
 *)
type wildcard_action =
  | Wildcard_fresh_tyvar
  | Wildcard_fresh_generic
  | Wildcard_higher_kinded_placeholder
  | Wildcard_require_explicit of decl_tparam
  | Wildcard_illegal

type visibility_behavior =
  | Always_expand_newtype
  | Expand_visible_newtype_only
  | Never_expand_newtype
[@@deriving show]

val is_default_visibility_behaviour : visibility_behavior -> bool

val default_visibility_behaviour : visibility_behavior

(** Tracks information about how a type was expanded *)
type expand_env = {
  type_expansions: Type_expansions.t;
  make_internal_opaque: bool;
      (** Localize internal classes outside their module as if newtypes i.e. opaque *)
  visibility_behavior: visibility_behavior;
  substs: locl_ty SMap.t;
  no_substs: SSet.t;
  this_ty: locl_ty;
  on_error: Typing_error.Reasons_callback.t option;
  wildcard_action: wildcard_action;
  ish_weakening: bool;
      (** If true, for refinement hints (is/as), approximate E by ~E & arraykey to account
       * for intish and stringish casts
       *)
}

val empty_expand_env : expand_env

val empty_expand_env_with_on_error :
  Typing_error.Reasons_callback.t -> expand_env

(** [add_type_expansion_check_cycles ety_env expansion] adds
  and [expansion] to [ety_env.expansions] and
  checks that that [expansion] hasn't already been done,
  i.e. wasn't already in ety_env.expansions. *)
val add_type_expansion_check_cycles :
  expand_env ->
  Type_expansions.expansion ->
  (expand_env, Type_expansions.cycle_reporter) result

(** Returns whether there was an attempt at expanding a cyclic type. *)
val cyclic_expansion : expand_env -> bool

val get_class_type : locl_phase ty -> (pos_id * exact * locl_ty list) option

val is_tyvar : locl_phase ty -> bool

val is_generic : 'a ty -> bool

val is_dynamic : 'a ty -> bool

val is_nonnull : 'a ty -> bool

val is_nothing : 'a ty -> bool

val is_wildcard : decl_phase ty -> bool

val is_fun : 'a ty -> bool

val is_any : 'a ty -> bool

val is_generic_equal_to : string -> 'a ty -> bool

val is_prim : Aast.tprim -> 'a ty -> bool

val is_union : 'a ty -> bool

val is_neg : locl_ty -> bool

(** Can the type be written down in a Hack program?
  - [false] result is sound but potentially incomplete;
  - [true] result is complete but potentially unsound.  *)
val is_denotable : locl_ty -> bool

(** Whether the given type is a union, intersection or option. *)
val is_union_or_inter_type : locl_ty -> bool

module Named_params : sig
  (** (Some name) iff fp is a named parameter *)
  val name_of_named_param : 'a fun_param -> string option

  (** (Some name) iff arg is named *)
  val name_of_arg : ('a, 'b) Aast_defs.argument -> string option
end

val this : Local_id.t

val make_tany : unit -> 'a ty_

(* Number of required parameters. Does not include optional, variadic, or
 * type-splat parameters
 *)
val arity_and_names_required : 'a fun_type -> int * SSet.t

val get_param_mode : Ast_defs.param_kind -> param_mode

module DependentKind : sig
  val to_string : dependent_type -> string

  val is_generic_dep_ty : string -> bool

  val strip_generic_dep_ty : string -> string option
end

module ShapeFieldMap : sig
  include module type of struct
    include TShapeMap
  end

  val map_and_rekey :
    'a shape_field_type TShapeMap.t ->
    (TShapeMap.key -> TShapeMap.key) ->
    ('a ty -> 'b ty) ->
    'b shape_field_type TShapeMap.t

  val map_env :
    ('a -> 'b ty -> 'a * 'c ty) ->
    'a ->
    'b shape_field_type TShapeMap.t ->
    'a * 'c shape_field_type TShapeMap.t

  val map_env_ty_err_opt :
    ('a -> 'b ty -> ('a * 'e option) * 'c ty) ->
    'a ->
    'b shape_field_type TShapeMap.t ->
    combine_ty_errs:('e list -> 'f) ->
    ('a * 'f) * 'c shape_field_type TShapeMap.t

  val map :
    ('a ty -> 'b ty) ->
    'a shape_field_type TShapeMap.t ->
    'b shape_field_type TShapeMap.t

  val iter :
    (TShapeMap.key -> 'a ty -> unit) -> 'a shape_field_type TShapeMap.t -> unit

  val iter_values : ('a ty -> unit) -> 'a shape_field_type TShapeMap.t -> unit
end

module ShapeFieldList : sig
  include module type of struct
    include Common.List
  end

  val map_env :
    'a ->
    'b shape_field_type list ->
    f:('a -> 'b ty -> 'a * 'c ty) ->
    'a * 'c shape_field_type Common.List.t
end

val is_suggest_mode : bool Hh_prelude.ref

val is_type_no_return : locl_ty_ -> bool

val get_ce_abstract : class_elt -> bool

val get_ce_final : class_elt -> bool

val get_ce_superfluous_override : class_elt -> bool

val get_ce_lsb : class_elt -> bool

(** Whether a class element comes from a `require extends`. *)
val get_ce_synthesized : class_elt -> bool

val get_ce_const : class_elt -> bool

val get_ce_lateinit : class_elt -> bool

val get_ce_readonly_prop_or_needs_concrete : class_elt -> bool

val get_ce_dynamicallycallable : class_elt -> bool

val get_ce_support_dynamic_type : class_elt -> bool

val get_ce_xhp_attr : class_elt -> xhp_attr option

val get_ce_safe_global_variable : class_elt -> bool

val get_ce_no_auto_likes : class_elt -> bool

val make_ce_flags :
  xhp_attr:xhp_attr option ->
  abstract:bool ->
  final:bool ->
  superfluous_override:bool ->
  lsb:bool ->
  synthesized:bool ->
  const:bool ->
  lateinit:bool ->
  dynamicallycallable:bool ->
  readonly_prop_or_needs_concrete:bool ->
  support_dynamic_type:bool ->
  needs_init:bool ->
  safe_global_variable:bool ->
  no_auto_likes:bool ->
  Typing_defs_flags.ClassElt.t

val class_elt_is_private_not_lsb : class_elt -> bool

val class_elt_is_private_or_protected_not_lsb : class_elt -> bool

val is_typeconst_type_abstract : typeconst_type -> bool

val is_arraykey : locl_ty -> bool

val is_string : locl_ty -> bool

module Attributes : sig
  val mem : string -> user_attribute Hh_prelude.List.t -> bool

  val find : string -> user_attribute Hh_prelude.List.t -> user_attribute option
end
