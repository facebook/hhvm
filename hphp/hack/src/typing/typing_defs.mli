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
    include Caml.Set.Make (CCR)
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
[@@deriving show]

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
[@@deriving show]

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

type requirement = Pos_or_decl.t * decl_ty

and abstract_typeconst = {
  atc_as_constraint: decl_ty option;
  atc_super_constraint: decl_ty option;
  atc_default: decl_ty option;
}

and concrete_typeconst = { tc_type: decl_ty }

and partially_abstract_typeconst = {
  patc_constraint: decl_ty;
  patc_type: decl_ty;
}

and typeconst =
  | TCAbstract of abstract_typeconst
  | TCConcrete of concrete_typeconst

and typeconst_type = {
  ttc_synthesized: bool;
  ttc_name: pos_id;
  ttc_kind: typeconst;
  ttc_origin: string;
  ttc_enforceable: Pos_or_decl.t * bool;
  ttc_reifiable: Pos_or_decl.t option;
  ttc_concretized: bool;
  ttc_is_ctx: bool;
}

and enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
  te_includes: decl_ty list;
}
[@@deriving show]

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
[@@deriving show]

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

  val ids : t -> string list

  val positions : t -> Pos_or_decl.t list
end

(** Tracks information about how a type was expanded *)
type expand_env = {
  type_expansions: Type_expansions.t;
  expand_visible_newtype: bool;
      (** Allow to expand visible `newtype`, i.e. opaque types defined in the current file.
          True by default. *)
  substs: locl_ty SMap.t;
  this_ty: locl_ty;
  on_error: Typing_error.Reasons_callback.t option;
  sub_wildcards: bool;
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

(** Returns [true] if both origins are available and identical.
    If this function returns [true], the two types that have
    the origins provided must be identical. *)
val same_type_origin : type_origin -> type_origin -> bool

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

val possibly_enforced_ty_compare :
  ?normalize_lists:bool ->
  'a ty possibly_enforced_ty ->
  'a ty possibly_enforced_ty ->
  Ppx_deriving_runtime.int

val ft_param_compare :
  ?normalize_lists:bool -> 'a ty fun_param -> 'a ty fun_param -> int

val ft_params_compare :
  ?normalize_lists:bool ->
  'a ty fun_param list ->
  'a ty fun_param list ->
  Ppx_deriving_runtime.int

val compare_locl_ty : ?normalize_lists:bool -> locl_ty -> locl_ty -> int

val compare_decl_ty : ?normalize_lists:bool -> decl_ty -> decl_ty -> int

val tyl_equal : 'a ty list -> 'a ty list -> bool

val class_id_con_ordinal : ('a, 'b) Aast.class_id_ -> int

val class_id_compare : ('a, 'b) Aast.class_id_ -> ('c, 'd) Aast.class_id_ -> int

val class_id_equal : ('a, 'b) Aast.class_id_ -> ('c, 'd) Aast.class_id_ -> bool

val has_member_compare :
  normalize_lists:bool -> has_member -> has_member -> Ppx_deriving_runtime.int

val can_index_compare :
  normalize_lists:bool -> can_index -> can_index -> Ppx_deriving_runtime.int

val destructure_compare :
  normalize_lists:bool -> destructure -> destructure -> Ppx_deriving_runtime.int

val constraint_ty_con_ordinal : constraint_type_ -> int

val constraint_ty_compare :
  ?normalize_lists:bool ->
  constraint_type ->
  constraint_type ->
  Ppx_deriving_runtime.int

val constraint_ty_equal :
  ?normalize_lists:bool -> constraint_type -> constraint_type -> bool

val ty_equal : ?normalize_lists:bool -> 'a ty -> 'a ty -> bool

val compare_exact : exact -> exact -> int

val equal_exact : exact -> exact -> bool

val equal_internal_type : internal_type -> internal_type -> bool

val equal_locl_ty : locl_ty -> locl_ty -> bool

val equal_locl_ty_ : locl_ty_ -> locl_ty_ -> bool

val is_type_no_return : locl_ty_ -> bool

val equal_decl_ty_ :
  decl_phase ty_ -> decl_phase ty_ -> Ppx_deriving_runtime.bool

val equal_decl_ty : decl_phase ty -> decl_phase ty -> Ppx_deriving_runtime.bool

val equal_shape_field_type :
  decl_phase shape_field_type -> decl_phase shape_field_type -> bool

val equal_decl_fun_type :
  decl_phase ty fun_type -> decl_phase ty fun_type -> bool

val non_public_ifc : ifc_fun_decl -> bool

val equal_decl_tyl :
  decl_phase ty Hh_prelude.List.t -> decl_phase ty Hh_prelude.List.t -> bool

val equal_decl_possibly_enforced_ty :
  decl_phase ty possibly_enforced_ty ->
  decl_phase ty possibly_enforced_ty ->
  bool

val equal_decl_fun_param :
  decl_phase ty fun_param -> decl_phase ty fun_param -> bool

val equal_decl_ft_params :
  decl_phase ty fun_params -> decl_phase ty fun_params -> bool

val equal_decl_ft_implicit_params :
  decl_ty fun_implicit_params -> decl_ty fun_implicit_params -> bool

val equal_typeconst : typeconst -> typeconst -> bool

val equal_enum_type : enum_type -> enum_type -> bool

val equal_decl_where_constraint :
  decl_phase ty * Ast_defs.constraint_kind * decl_phase ty ->
  decl_phase ty * Ast_defs.constraint_kind * decl_phase ty ->
  bool

val equal_decl_tparam : decl_phase ty tparam -> decl_phase ty tparam -> bool

val equal_typedef_type : typedef_type -> typedef_type -> bool

val equal_fun_elt : fun_elt -> fun_elt -> bool

val equal_const_decl : const_decl -> const_decl -> bool

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
