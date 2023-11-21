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
}
[@@deriving eq, show]

type class_elt = {
  ce_visibility: ce_visibility;
  ce_type: decl_ty Hh_prelude.Lazy.t;
  ce_origin: string;
  ce_deprecated: string option;
  ce_pos: Pos_or_decl.t Hh_prelude.Lazy.t;
  ce_flags: Typing_defs_flags.ClassElt.t;
}
[@@deriving show]

type fun_elt = {
  fe_deprecated: string option;
  fe_module: Ast_defs.id option;
  fe_internal: bool;
  fe_type: decl_ty;
  fe_pos: Pos_or_decl.t;
  fe_php_std_lib: bool;
  fe_support_dynamic_type: bool;
  fe_no_auto_dynamic: bool;
  fe_no_auto_likes: bool;
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
  cc_refs: class_const_ref list;
}
[@@deriving show]

type module_reference =
  | MRGlobal
  | MRPrefix of string
  | MRExact of string
[@@deriving show]

type module_def_type = {
  mdt_pos: Pos_or_decl.t;
  mdt_exports: module_reference list option;
  mdt_imports: module_reference list option;
}
[@@deriving show]

type requirement = Pos_or_decl.t * decl_ty [@@deriving show]

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

type typedef_type = {
  td_module: Ast_defs.id option;
  td_pos: Pos_or_decl.t;
  td_vis: Aast.typedef_visibility;
  td_tparams: decl_tparam list;
  td_as_constraint: decl_ty option;
  td_super_constraint: decl_ty option;
  td_type: decl_ty;
  td_is_ctx: bool;
  td_attributes: user_attribute list;
  td_internal: bool;
  td_docs_url: string option;
}
[@@deriving eq, show]

type phase_ty =
  | DeclTy of decl_ty
  | LoclTy of locl_ty

type deserialization_error =
  | Wrong_phase of string
  | Not_supported of string
  | Deserialization_error of string
[@@deriving show]

module Type_expansions : sig
  (** A list of the type defs and type access we have expanded thus far. Used
      to prevent entering into a cycle when expanding these types. *)
  type t

  val empty : t

  (** If we are expanding the RHS of a type definition, [report_cycle] contains
      the position and id of the LHS. This way, if the RHS expands at some point
      to the LHS id, we are able to report a cycle. *)
  val empty_w_cycle_report : report_cycle:(Pos.t * string) option -> t

  (** Returns:
    - [None] if there was no cycle
    - [Some None] if there was a cycle which did not involve the first
      type expansion, i.e. error reporting should be done elsewhere
    - [Some (Some pos)] if there was a cycle involving the first type
      expansion in which case an error should be reported at [pos]. *)
  val add_and_check_cycles :
    t -> Pos_or_decl.t * string -> t * Pos.t option option

  val ids : t -> string list

  val positions : t -> Pos_or_decl.t list
end

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

(** Tracks information about how a type was expanded *)
type expand_env = {
  type_expansions: Type_expansions.t;
  expand_visible_newtype: bool;
      (** Allow to expand visible `newtype`, i.e. opaque types defined in the current file.
          True by default. *)
  substs: locl_ty SMap.t;
  this_ty: locl_ty;
  on_error: Typing_error.Reasons_callback.t option;
  wildcard_action: wildcard_action;
}

val empty_expand_env : expand_env

val empty_expand_env_with_on_error :
  Typing_error.Reasons_callback.t -> expand_env

(** Returns:
    - [None] if there was no cycle
    - [Some None] if there was a cycle which did not involve the first
      type expansion, i.e. error reporting should be done elsewhere
    - [Some (Some pos)] if there was a cycle involving the first type
      expansion in which case an error should be reported at [pos]. *)
val add_type_expansion_check_cycles :
  expand_env -> Pos_or_decl.t * string -> expand_env * Pos.t option option

(** Returns whether there was an attempt at expanding a cyclic type. *)
val cyclic_expansion : expand_env -> bool

val get_var : locl_phase ty -> Ident.t option

val get_class_type : locl_phase ty -> (pos_id * exact * locl_ty list) option

val get_var_i : internal_type -> Ident.t option

val is_tyvar : locl_phase ty -> bool

val is_tyvar_i : internal_type -> bool

val is_var_v : locl_phase ty -> Ident.t -> bool

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

val is_constraint_type_union : constraint_type -> bool

val is_has_member : constraint_type -> bool

(** Can the type be written down in a Hack program?
  - [false] result is sound but potentially incomplete;
  - [true] result is complete but potentially unsound.  *)
val is_denotable : locl_ty -> bool

val show_phase_ty : 'a -> string

val pp_phase_ty : 'a -> 'b -> Base.unit

val is_locl_type : internal_type -> bool

val reason : internal_type -> locl_phase Reason.t_

val is_constraint_type : internal_type -> bool

(** Whether the given type is a union, intersection or option. *)
val is_union_or_inter_type : locl_ty -> bool

module InternalType : sig
  val get_var : internal_type -> Ident.t option

  val is_var_v : internal_type -> v:Ident.t -> bool

  val is_not_var_v : internal_type -> v:Ident.t -> bool
end

val this : Local_id.t

val make_tany : unit -> 'a ty_

val arity_min : 'a fun_type -> int

val get_param_mode : Ast_defs.param_kind -> param_mode

module DependentKind : sig
  val to_string : dependent_type -> string

  val is_generic_dep_ty : string -> bool
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

val get_ce_readonly_prop : class_elt -> bool

val get_ce_dynamicallycallable : class_elt -> bool

val get_ce_support_dynamic_type : class_elt -> bool

val get_ce_xhp_attr : class_elt -> xhp_attr option

val get_ce_safe_global_variable : class_elt -> bool

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
  readonly_prop:bool ->
  support_dynamic_type:bool ->
  needs_init:bool ->
  safe_global_variable:bool ->
  Typing_defs_flags.ClassElt.t

val class_elt_is_private_not_lsb : class_elt -> bool

val class_elt_is_private_or_protected_not_lsb : class_elt -> bool

val error_Tunapplied_alias_in_illegal_context : unit -> 'a

val is_typeconst_type_abstract : typeconst_type -> bool

module Attributes : sig
  val mem : string -> user_attribute Hh_prelude.List.t -> bool

  val find : string -> user_attribute Hh_prelude.List.t -> user_attribute option
end
