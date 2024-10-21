(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pos_id = Pos_or_decl.t * Ast_defs.id_ [@@deriving eq, hash, ord, show]

type arg_position =
  | Aonly
  | Afirst
  | Asecond
[@@deriving eq, show]

type expr_dep_type_reason =
  | ERexpr of Expression_id.t
  | ERstatic
  | ERclass of string
  | ERparent of string
  | ERself of string
  | ERpu of string
[@@deriving eq, show]

type blame_source =
  | BScall
  | BSlambda
  | BSassignment
  | BSout_of_scope
[@@deriving eq, show]

type blame = Blame of Pos.t * blame_source [@@deriving eq, show]

(* All the possible types, reason is a trace of why a type
   was inferred in a certain way.

   Types exists in two phases. Phase one is 'decl_phase', meaning it is a type that
   was declared in user code. Phase two is 'locl_phase', meaning it is a type that is
   inferred via local inference.
*)
(* create private types to represent the different type phases *)
type decl_phase = private DeclPhase [@@deriving eq, hash, show]

type locl_phase = private LoclPhase [@@deriving eq, hash, show]

(* The phase below helps enforce that only Pos_or_decl.t positions end up in the heap.
 * To enforce that, any reason taking a Pos.t should be a locl_phase t_
 * to prevent a decl ty from using it.
 * Reasons used for decl types should be 'phase t_ so that they can be localized
 * to be used in the localized version of the type. *)

type field_kind =
  | Absent
  | Optional
  | Required
[@@deriving hash]

type ctor_kind =
  | Ctor_class
  | Ctor_newtype

(** The reason why something is expected to have a certain type *)
type _ t_ [@@deriving hash, show]

type t = locl_phase t_ [@@deriving show]

val to_json : 'a t_ -> Hh_json.json

val debug_reason : sub:locl_phase t_ -> super:locl_phase t_ -> 'a Explanation.t

val debug_derivation :
  sub:locl_phase t_ -> super:locl_phase t_ -> 'a Explanation.t

val explain :
  sub:locl_phase t_ ->
  super:locl_phase t_ ->
  complexity:int ->
  Pos_or_decl.t Explanation.t

type decl_t = decl_phase t_

val localize : decl_t -> t

val reverse_flow : t -> t

(** Translate a reason to a (pos, string) list, suitable for error_l. This
    previously returned a string, however the need to return multiple lines with
    multiple locations meant that it needed to more than convert to a string *)
val to_string : string -> 'phase t_ -> (Pos_or_decl.t * string) list

val to_pos : 'phase t_ -> Pos_or_decl.t

val to_constructor_string : 'phase t_ -> string

type ureason =
  | URnone
  | URassign
  | URassign_inout
  | URhint
  | URreturn
  | URforeach
  | URthrow
  | URvector
  | URkey of string
  | URvalue of string
  | URawait
  | URyield
  | URxhp of string * string  (** Name of XHP class, Name of XHP attribute *)
  | URxhp_spread
  | URindex of string
  | URelement of string
  | URparam
  | URparam_inout
  | URarray_value
  | URpair_value
  | URtuple_access
  | URpair_access
  | URnewtype_cstr
  | URclass_req
  | URenum
  | URenum_include
  | URenum_cstr
  | URenum_underlying
  | URenum_incompatible_cstr
  | URtypeconst_cstr
  | URsubsume_tconst_cstr
  | URsubsume_tconst_assign
  | URclone
  | URusing
  | URstr_concat
  | URstr_interp
  | URdynamic_prop
  | URlabel
[@@deriving show]

val index_array : ureason

val index_tuple : ureason

val index_class : string -> ureason

val set_element : string -> ureason

val string_of_ureason : ureason -> string

val none : 'phase t_

val witness : Pos.t -> t

val witness_from_decl : Pos_or_decl.t -> 'phase t_

(* Used as an index into a vector-like
   array or string. Position of indexing,
   reason for the indexed type *)
val idx : Pos.t * t -> t

(* Used as an index, in the Vector case *)
val idx_vector : Pos.t -> t

val idx_vector_from_decl : Pos_or_decl.t -> 'phase t_

(* Because it is iterated in a foreach loop *)
val foreach : Pos.t -> t

(* Because it is iterated "await as" in foreach *)
val asyncforeach : Pos.t -> t

val arith : Pos.t -> t

val arith_ret : Pos.t -> t

(* pos, arg float typing reason, arg position *)
val arith_ret_float : Pos.t * t * arg_position -> t

(* pos, arg num typing reason, arg position *)
val arith_ret_num : Pos.t * t * arg_position -> t

val arith_ret_int : Pos.t -> t

val arith_dynamic : Pos.t -> t

val bitwise_dynamic : Pos.t -> t

val incdec_dynamic : Pos.t -> t

val comp : Pos.t -> t

val concat_ret : Pos.t -> t

val logic_ret : Pos.t -> t

val bitwise : Pos.t -> t

val bitwise_ret : Pos.t -> t

val no_return : Pos.t -> t

val no_return_async : Pos.t -> t

val ret_fun_kind : Pos.t * Ast_defs.fun_kind -> t

val ret_fun_kind_from_decl : Pos_or_decl.t * Ast_defs.fun_kind -> 'phase t_

val hint : Pos_or_decl.t -> 'phase t_

val throw : Pos.t -> t

val placeholder : Pos.t -> t

val ret_div : Pos.t -> t

val yield_gen : Pos.t -> t

val yield_asyncgen : Pos.t -> t

val yield_asyncnull : Pos.t -> t

val yield_send : Pos.t -> t

val lost_info : string * t * blame -> t

val format : Pos.t * string * t -> t

val class_class : Pos_or_decl.t * string -> 'phase t_

val unknown_class : Pos.t -> t

val var_param : Pos.t -> t

val var_param_from_decl : Pos_or_decl.t -> 'phase t_

(* splat pos, fun def pos, number of args before splat *)
val unpack_param : Pos.t * Pos_or_decl.t * int -> t

val inout_param : Pos_or_decl.t -> 'phase t_

val instantiate : 'phase t_ * string * 'phase t_ -> 'phase t_

val typeconst :
  'phase t_ * (Pos_or_decl.t * string) * string Lazy.t * 'phase t_ -> 'phase t_

val type_access : t * (t * string Lazy.t) list -> t

val expr_dep_type :
  'phase t_ * Pos_or_decl.t * expr_dep_type_reason -> 'phase t_

(* ?-> operator is used *)
val nullsafe_op : Pos.t -> t

val tconst_no_cstr : pos_id -> 'phase t_

val predicated : Pos.t * string -> t

val is_refinement : Pos.t -> t

val as_refinement : Pos.t -> t

val equal : Pos.t -> t

val varray_or_darray_key : Pos_or_decl.t -> 'phase t_

val vec_or_dict_key : Pos_or_decl.t -> 'phase t_

val using : Pos.t -> t

val dynamic_prop : Pos.t -> t

val dynamic_call : Pos.t -> t

val dynamic_construct : Pos.t -> t

val idx_dict : Pos.t -> t

val idx_set_element : Pos.t -> t

val missing_optional_field : Pos_or_decl.t * string -> 'phase t_

val unset_field : Pos.t * string -> t

val contravariant_generic : t * string -> t

val invariant_generic : t * string -> t

val regex : Pos.t -> t

val implicit_upper_bound : Pos_or_decl.t * string -> 'phase t_

val type_variable : Pos.t -> Tvid.t -> t

val type_variable_generics : Pos.t * string * string -> Tvid.t -> t

val type_variable_error : Pos.t -> Tvid.t -> t

val global_type_variable_generics : Pos_or_decl.t * string * string -> 'phase t_

val solve_fail : Pos_or_decl.t -> 'phase t_

val cstr_on_generics : Pos_or_decl.t * pos_id -> 'phase t_

val lambda_param : Pos.t * t -> t

val shape : Pos.t * string -> t

val shape_literal : Pos.t -> t

val enforceable : Pos_or_decl.t -> 'phase t_

val destructure : Pos.t -> t

val tuple_from_splat : Pos_or_decl.t -> 'phase t_

val key_value_collection_key : Pos.t -> t

val global_class_prop : Pos_or_decl.t -> 'phase t_

val global_fun_param : Pos_or_decl.t -> 'phase t_

val global_fun_ret : Pos_or_decl.t -> 'phase t_

val splice : Pos.t -> t

val et_boolean : Pos.t -> t

val default_capability : Pos_or_decl.t -> 'phase t_

val concat_operand : Pos.t -> t

val interp_operand : Pos.t -> t

val dynamic_coercion : t -> t

val support_dynamic_type : Pos_or_decl.t -> 'phase t_

val dynamic_partial_enforcement : Pos_or_decl.t * string * t -> t

val rigid_tvar_escape : Pos.t * string * string * t -> t

val opaque_type_from_module : Pos_or_decl.t * string * t -> t

val missing_class : Pos.t -> t

val invalid : 'phase t_

val captured_like : Pos.t -> t

val pessimised_inout : Pos_or_decl.t -> 'phase t_

val pessimised_return : Pos_or_decl.t -> 'phase t_

val pessimised_prop : Pos_or_decl.t -> 'phase t_

val unsafe_cast : Pos.t -> t

val pattern : Pos.t -> t

val join_point : Pos.t -> t

val axiom_extends :
  child:locl_phase t_ -> ancestor:locl_phase t_ -> locl_phase t_

val flow_assign : rhs:locl_phase t_ -> lval:locl_phase t_ -> locl_phase t_

val flow_local : def:locl_phase t_ -> use:locl_phase t_ -> locl_phase t_

val flow_call : def:locl_phase t_ -> use:locl_phase t_ -> locl_phase t_

val flow_prop_access : def:locl_phase t_ -> use:locl_phase t_ -> locl_phase t_

val flow_return_expr : expr:locl_phase t_ -> ret:locl_phase t_ -> locl_phase t_

val flow_return_hint : hint:locl_phase t_ -> use:locl_phase t_ -> locl_phase t_

val flow_param_hint : hint:locl_phase t_ -> param:locl_phase t_ -> locl_phase t_

val solved :
  Tvid.t -> solution:locl_phase t_ -> in_:locl_phase t_ -> locl_phase t_

val axiom_upper_bound :
  bound:locl_phase t_ -> of_:locl_phase t_ -> locl_phase t_

val axiom_lower_bound :
  bound:locl_phase t_ -> of_:locl_phase t_ -> locl_phase t_

val trans_lower_bound :
  bound:locl_phase t_ -> of_:locl_phase t_ -> locl_phase t_

val definition : Pos_or_decl.t -> locl_phase t_ -> locl_phase t_

(** Record the decomposition of a covariant type parameter when simplifying a
    subtype constraint between two type constructors *)
val prj_ctor_co :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  ctor_kind ->
  string ->
  int ->
  bool ->
  locl_phase t_

(** Record the decomposition of a contravariant type parameter when simplifying a
   subtype constraint between two classes *)
val prj_ctor_contra :
  sub:locl_phase t_ ->
  super:locl_phase t_ ->
  super_prj:locl_phase t_ ->
  ctor_kind ->
  string ->
  int ->
  bool ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two negated types
    into a constraint between the two inner types *)
val prj_neg :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two supportdyn types
    into a constraint between the two non-dynamic types *)
val prj_supportdyn :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two nullable types
    into a constraint between the two inner non-null types *)
val prj_nullable :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two tuple types
    into a constraint between the two types at index [idx] *)
val prj_tuple :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  int ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two shape types
    into a constraint between the two types for a given field [lbl]; since
    the subtype relationship may depend on the field 'kind' in the sub- and
    supertype, we also record this information *)
val prj_shape :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  string ->
  kind_sub:field_kind ->
  kind_super:field_kind ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two function types
    into a constraint between the types of a given parameter; when the function
    has variadic arguments the indices may differ so we explicitly record the
    index in both sub- and supertype *)
val prj_fn_param :
  sub:locl_phase t_ ->
  super:locl_phase t_ ->
  super_prj:locl_phase t_ ->
  idx_sub:int ->
  idx_super:int ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two function types
    into a constraint between the types of a given inout parameter treated
    as covariant *)
val prj_fn_param_inout_co :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  idx_sub:int ->
  idx_super:int ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two function types
    into a constraint between the types of a given inout parameter treated
    as contravariant *)
val prj_fn_param_inout_contra :
  sub:locl_phase t_ ->
  super:locl_phase t_ ->
  super_prj:locl_phase t_ ->
  idx_sub:int ->
  idx_super:int ->
  locl_phase t_

(** Record the decomposition of a subtype contraint between two function types
    into a constraint between the return types *)
val prj_fn_ret :
  sub:locl_phase t_ ->
  sub_prj:locl_phase t_ ->
  super:locl_phase t_ ->
  locl_phase t_

(** Record the decomposition of a subtype constraint between a union type in
    subtype position and some other type into another contraint between some
    member of the union and the other type *)
val prj_union_sub : sub:locl_phase t_ -> sub_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a union type in
    supertype position and some other type into another contraint between some
    member of the union and the other type *)
val prj_union_super :
  super:locl_phase t_ -> super_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a intersection type
    in subtype position and some other type into another contraint between some
    member of the union and the other type *)
val prj_inter_sub : sub:locl_phase t_ -> sub_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a intersection type
    in supertype position and some other type into another contraint between
    some member of the union and the other type *)
val prj_inter_super :
  super:locl_phase t_ -> super_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a negated type in
    subtype position and some other type into another contraint between some
    member of the union and the other type *)
val prj_neg_sub : sub:locl_phase t_ -> sub_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a negated type in
    supertype position and some other type into another contraint between some
    member of the union and the other type *)
val prj_neg_super :
  super:locl_phase t_ -> super_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a nullable type in
    subtype position and some other type into another contraint between either
    the null or non-null part of the nullable type and the other type *)
val prj_nullable_sub :
  sub:locl_phase t_ -> sub_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a nullable type in
    supertype position and some other type into another contraint between either
    the null or non-null part of the nullable type and the other type *)
val prj_nullable_super :
  super:locl_phase t_ -> super_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a num type in
    subtype position and some other type into another contraint between either
    the int or float part of the num type and the other type *)
val prj_num_sub : sub:locl_phase t_ -> sub_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between a num type in
    supertype position and some other type into another contraint between either
    the int or float part of the num type and the other type *)
val prj_num_super :
  super:locl_phase t_ -> super_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between an arraykey type in
    subtype position and some other type into another contraint between either
    the int or string part of the arraykey type and the other type *)
val prj_arraykey_sub :
  sub:locl_phase t_ -> sub_prj:locl_phase t_ -> locl_phase t_

(** Record the decomposition of a subtype constraint between an arraykey type in
    supertype position and some other type into another contraint between either
    the int or string part of the arraykey type and the other type *)
val prj_arraykey_super :
  super:locl_phase t_ -> super_prj:locl_phase t_ -> locl_phase t_

val missing_field : t

val pessimised_this : Pos_or_decl.t -> 'phase t_

val illegal_recursive_type : Pos_or_decl.t -> string -> 'phase t_

val compare : 'phase t_ -> 'phase t_ -> int

val map_pos :
  (Pos.t -> Pos.t) -> (Pos_or_decl.t -> Pos_or_decl.t) -> 'phase t_ -> 'phase t_

val force_lazy_values : t -> t

module Predicates : sig
  val is_none : t -> bool

  val is_opaque_type_from_module : t -> bool

  val is_instantiate : t -> bool

  val is_hint : t -> bool

  val is_captured_like : t -> bool

  val unpack_expr_dep_type_opt :
    t -> (t * Pos_or_decl.t * expr_dep_type_reason) option

  val unpack_unpack_param_opt : t -> (Pos.t * Pos_or_decl.t * int) option

  val unpack_cstr_on_generics_opt : t -> (Pos_or_decl.t * pos_id) option

  val unpack_shape_literal_opt : t -> Pos.t option
end
