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
  ce_flags: int;
}
[@@deriving show]

type fun_elt = {
  fe_deprecated: string option;
  fe_type: decl_ty;
  fe_pos: Pos_or_decl.t;
  fe_php_std_lib: bool;
}
[@@deriving show]

type class_const = {
  cc_synthesized: bool;
  cc_abstract: bool;
  cc_pos: Pos_or_decl.t;
  cc_type: decl_ty;
  cc_origin: string;
  cc_refs: class_const_ref list;
}
[@@deriving show]

type record_field_req =
  | ValueRequired
  | HasDefaultValue
[@@deriving show]

type record_def_type = {
  rdt_name: pos_id;
  rdt_extends: pos_id option;
  rdt_fields: (pos_id * record_field_req) list;
  rdt_abstract: bool;
  rdt_pos: Pos_or_decl.t;
}
[@@deriving show]

type requirement = Pos_or_decl.t * decl_ty

and class_type = {
  tc_need_init: bool;
  tc_members_fully_known: bool;
  tc_abstract: bool;
  tc_final: bool;
  tc_const: bool;
  tc_deferred_init_members: SSet.t;
  tc_kind: Ast_defs.class_kind;
  tc_is_xhp: bool;
  tc_has_xhp_keyword: bool;
  tc_is_disposable: bool;
  tc_name: string;
  tc_pos: Pos_or_decl.t;
  tc_tparams: decl_tparam list;
  tc_where_constraints: decl_where_constraint list;
  tc_consts: class_const SMap.t;
  tc_typeconsts: typeconst_type SMap.t;
  tc_props: class_elt SMap.t;
  tc_sprops: class_elt SMap.t;
  tc_methods: class_elt SMap.t;
  tc_smethods: class_elt SMap.t;
  tc_construct: class_elt option * consistent_kind;
  tc_ancestors: decl_ty SMap.t;
  tc_implements_dynamic: bool;
  tc_req_ancestors: requirement list;
  tc_req_ancestors_extends: SSet.t;
  tc_extends: SSet.t;
  tc_enum_type: enum_type option;
  tc_sealed_whitelist: SSet.t option;
  tc_decl_errors: Errors.t option; [@opaque]
}

and typeconst_abstract_kind =
  | TCAbstract of decl_ty option
  | TCPartiallyAbstract
  | TCConcrete

and typeconst_type = {
  ttc_abstract: typeconst_abstract_kind;
  ttc_synthesized: bool;
  ttc_name: pos_id;
  ttc_as_constraint: decl_ty option;
  ttc_super_constraint: decl_ty option;
  ttc_type: decl_ty option;
  ttc_origin: string;
  ttc_enforceable: Pos_or_decl.t * bool;
  ttc_reifiable: Pos_or_decl.t option;
  ttc_concretized: bool;
}

and enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
  te_includes: decl_ty list;
  te_enum_class: bool;
}
[@@deriving show]

type typedef_type = {
  td_pos: Pos_or_decl.t;
  td_vis: Aast.typedef_visibility;
  td_tparams: decl_tparam list;
  td_constraint: decl_ty option;
  td_type: decl_ty;
}
[@@deriving show]

val is_enum_class : enum_type option -> bool

type phase_ty =
  | DeclTy of decl_ty
  | LoclTy of locl_ty

type deserialization_error =
  | Wrong_phase of string
  | Not_supported of string
  | Deserialization_error of string

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
  substs: locl_ty SMap.t;
  this_ty: locl_ty;
  on_error: Errors.error_from_reasons_callback;
}

(** Returns:
    - [None] if there was no cycle
    - [Some None] if there was a cycle which did not involve the first
      type expansion, i.e. error reporting should be done elsewhere
    - [Some (Some pos)] if there was a cycle involving the first type
      expansion in which case an error should be reported at [pos]. *)
val add_type_expansion_check_cycles :
  expand_env -> Pos_or_decl.t * string -> expand_env * Pos.t option option

val get_var : 'a ty -> Ident.t option

val get_class_type : locl_phase ty -> (pos_id * exact * locl_ty list) option

val get_var_i : internal_type -> Ident.t option

val is_tyvar : 'a ty -> bool

val is_var_v : 'a ty -> Ident.t -> bool

val is_generic : 'a ty -> bool

val is_dynamic : 'a ty -> bool

val is_nonnull : 'a ty -> bool

val is_fun : 'a ty -> bool

val is_any : 'a ty -> bool

val is_generic_equal_to : string -> 'a ty -> bool

val is_prim : Aast.tprim -> 'a ty -> bool

val is_union : 'a ty -> bool

val is_constraint_type_union : constraint_type -> bool

val is_has_member : constraint_type -> bool

val show_phase_ty : 'a -> string

val pp_phase_ty : 'a -> 'b -> Base.unit

val is_locl_type : internal_type -> bool

val reason : internal_type -> locl_phase Reason.t_

val is_constraint_type : internal_type -> bool

val is_union_or_inter_type : locl_ty -> bool

module InternalType : sig
  val get_var : internal_type -> Ident.t option

  val is_var_v : internal_type -> v:Ident.t -> bool

  val is_not_var_v : internal_type -> v:Ident.t -> bool
end

val this : Local_id.t

val make_tany : unit -> 'a ty_

val arity_min : 'a fun_type -> int

val get_param_mode : Ast_defs.param_kind option -> param_mode

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

val class_id_con_ordinal : ('a, 'b, 'c, 'd) Aast.class_id_ -> int

val class_id_compare :
  ('a, 'b, 'c, 'd) Aast.class_id_ -> ('e, 'f, 'g, 'h) Aast.class_id_ -> int

val class_id_equal :
  ('a, 'b, 'c, 'd) Aast.class_id_ -> ('e, 'f, 'g, 'h) Aast.class_id_ -> bool

val has_member_compare :
  normalize_lists:bool -> has_member -> has_member -> Ppx_deriving_runtime.int

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

val equal_internal_type : internal_type -> internal_type -> bool

val equal_locl_ty : locl_ty -> locl_ty -> bool

val equal_locl_ty_ : locl_ty_ -> locl_ty_ -> bool

val equal_locl_fun_arity : 'a ty fun_type -> 'a ty fun_type -> bool

val is_type_no_return : locl_ty_ -> bool

val equal_decl_ty_ :
  decl_phase ty_ -> decl_phase ty_ -> Ppx_deriving_runtime.bool

val equal_decl_ty : decl_phase ty -> decl_phase ty -> Ppx_deriving_runtime.bool

val equal_shape_field_type :
  decl_phase shape_field_type -> decl_phase shape_field_type -> bool

val equal_decl_fun_arity :
  decl_phase ty fun_type -> decl_phase ty fun_type -> bool

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

val equal_typeconst_abstract_kind :
  typeconst_abstract_kind -> typeconst_abstract_kind -> bool

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

val get_ce_override : class_elt -> bool

val get_ce_lsb : class_elt -> bool

val get_ce_synthesized : class_elt -> bool

val get_ce_const : class_elt -> bool

val get_ce_lateinit : class_elt -> bool

val get_ce_readonly_prop : class_elt -> bool

val get_ce_dynamicallycallable : class_elt -> bool

val get_ce_sound_dynamic_callable : class_elt -> bool

val xhp_attr_to_ce_flags : xhp_attr option -> Hh_prelude.Int.t

val flags_to_xhp_attr : Hh_prelude.Int.t -> xhp_attr option

val get_ce_xhp_attr : class_elt -> xhp_attr option

val make_ce_flags :
  xhp_attr:xhp_attr option ->
  abstract:bool ->
  final:bool ->
  override:bool ->
  lsb:bool ->
  synthesized:bool ->
  const:bool ->
  lateinit:bool ->
  dynamicallycallable:bool ->
  readonly_prop:bool ->
  sound_dynamic_callable:bool ->
  Hh_prelude.Int.t

val error_Tunapplied_alias_in_illegal_context : unit -> 'a

module Attributes : sig
  val mem : string -> user_attribute Hh_prelude.List.t -> bool

  val find : string -> user_attribute Hh_prelude.List.t -> user_attribute option
end
