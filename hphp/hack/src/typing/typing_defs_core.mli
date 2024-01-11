(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Reason = Typing_reason

type pos_id = Reason.pos_id [@@deriving eq, ord, hash, show]

type ce_visibility =
  | Vpublic
  | Vprivate of string
  | Vprotected of string
  (* When we construct `Vinternal`, we are guaranteed to be inside a module *)
  | Vinternal of string
[@@deriving eq, ord, show]

type cross_package_decl = string option [@@deriving eq, ord]

(* All the possible types, reason is a trace of why a type
   was inferred in a certain way.

   Types exists in two phases. Phase one is 'decl', meaning it is a type that
   was declared in user code. Phase two is 'locl', meaning it is a type that is
   inferred via local inference.
*)
(* create private types to represent the different type phases *)
type decl_phase = Typing_reason.decl_phase [@@deriving eq, show]

type locl_phase = Typing_reason.locl_phase [@@deriving eq, show]

type val_kind =
  | Lval
  | LvalSubexpr
  | Other
[@@deriving eq]

type fun_tparams_kind =
  | FTKtparams
      (** If ft_tparams is empty, the containing fun_type is a concrete function type.
      Otherwise, it is a generic function and ft_tparams specifies its type parameters. *)
  | FTKinstantiated_targs
      (** The containing fun_type is a concrete function type which is an
      instantiation of a generic function with at least one reified type
      parameter. This means that the function requires explicit type arguments
      at every invocation, and ft_tparams specifies the type arguments with
      which the generic function was instantiated, as well as whether each
      explicit type argument must be reified. *)
[@@deriving eq]

(** The origin of a type is a succinct key that is unique to the
    type containing it. Consequently, two types with the same
    origin are necessarily identical. Any change to a type with
    origin needs to come with a *reset* of its origin. For example,
    all type mappers have to reset origins to [Missing_origin]. *)
type type_origin =
  | Missing_origin
      (** When we do not have any origin for the type. It is always
          correct to use [Missing_origin]; so when in doubt, use it. *)
  | From_alias of string
      (** A type with origin [From_alias orig] is equivalent to
           the expansion of the alias [orig]. *)
[@@deriving eq, ord, show]

type pos_string = Pos_or_decl.t * string [@@deriving eq, ord, show]

type pos_byte_string = Pos_or_decl.t * Ast_defs.byte_string
[@@deriving eq, ord, show]

(** This is similar to Aast.shape_field_name, but contains Pos_or_decl.t
    instead of Pos.t. Aast.shape_field_name is used in shape expressions,
    while this is used in shape types. *)
type tshape_field_name =
  | TSFlit_int of pos_string
  | TSFlit_str of pos_byte_string
  | TSFclass_const of pos_id * pos_string
[@@deriving eq, ord, show]

(** This is similar to Aast.ShapeField, but contains Pos_or_decl.t
    instead of Pos.t. Aast.ShapeField is used in shape expressions,
    while this is used in shape types. *)
module TShapeField : sig
  type t = tshape_field_name [@@deriving show]

  val pos : t -> Pos_or_decl.t

  val name : t -> string

  val of_ast : (Pos.t -> Pos_or_decl.t) -> Ast_defs.shape_field_name -> t

  (** Compare but ignore positions. *)
  val compare : t -> t -> int

  (** Whether two values are equal. Ignore positions. *)
  val equal : t -> t -> bool
end

(** This is similar to Ast_defs.ShapeMap, but contains Pos_or_decl.t
    instead of Pos.t. Ast_defs.ShapeMap is used in shape expressions,
    while this is used in shape types. *)
module TShapeMap : sig
  include WrappedMap.S with type key = TShapeField.t

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val map_and_rekey : 'a t -> (key -> key) -> ('a -> 'b) -> 'b t
end

module TShapeSet : Stdlib.Set.S with type elt = TShapeField.t

type param_mode =
  | FPnormal
  | FPinout
[@@deriving eq, show]

type xhp_attr = Xhp_attribute.t [@@deriving eq, show]

(* Denotes the categories of requirements we apply to constructor overrides.
 *
 * In the default case, we use Inconsistent. If a class has <<__ConsistentConstruct>>,
 * or if it inherits a class that has <<__ConsistentConstruct>>, we use inherited.
 * If we have a new final class that doesn't extend from <<__ConsistentConstruct>>,
 * then we use Final. Only classes that are Inconsistent or Final can have reified
 * generics. *)
type consistent_kind =
  | Inconsistent
  | ConsistentConstruct
  | FinalClass
[@@deriving eq, show]

type dependent_type =
  (* A reference to some expression. For example:
   *
   *  $x->foo()
   *
   *  The expression $x would have a certain ID.
   *  The expression $x->foo() would have a different one.
   *)
  | DTexpr of Expression_id.t
[@@deriving eq, ord, show]

type user_attribute_param =
  | Classname of string
  | EnumClassLabel of string
  | String of Ast_defs.byte_string
  | Int of string
[@@deriving eq, hash, show]

val user_attribute_param_to_string : user_attribute_param -> string

type user_attribute = {
  ua_name: pos_id;
  ua_params: user_attribute_param list;
}
[@@deriving eq, show]

type 'ty tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: pos_id;
  tp_tparams: 'ty tparam list;
  tp_constraints: (Ast_defs.constraint_kind * 'ty) list;
  tp_reified: Aast.reify_kind;
  tp_user_attributes: user_attribute list;
}
[@@deriving eq, show]

type 'ty where_constraint = 'ty * Ast_defs.constraint_kind * 'ty
[@@deriving eq, show]

type enforcement =
  | Unenforced  (** The consumer doesn't enforce the type at runtime *)
  | Enforced  (** The consumer enforces the type at runtime *)
[@@deriving eq, show, ord]

(** Negation types represent the type of values that fail an `is` test
    for either a primitive type, or a class-ish type C<_> *)
type neg_type =
  | Neg_prim of Aast.tprim  (** The negation of a primitive type *)
  | Neg_class of pos_id
      (** The negation of a class. If we think of types as denoting sets
       of values, then (Neg_class C) is complement (Union tyl. C<tyl>), that is
       all values that are not in C<t1, ..., tn> for any application of C to type
       arguments. *)
[@@deriving hash, show]

(** Because Tfun is currently used as both a decl and locl ty, without this,
  the HH\Contexts\defaults alias must be stored in shared memory for a
  decl Tfun record. We can eliminate this if the majority of usages end up
  explicit or if we separate decl and locl Tfuns. *)
type 'ty capability =
  | CapDefaults of Pos_or_decl.t  (** Should not be used for lambda inference *)
  | CapTy of 'ty
[@@deriving hash, show]

(** Companion to fun_params type, intended to consolidate checking of
  implicit params for functions. *)
type 'ty fun_implicit_params = { capability: 'ty capability }
[@@deriving hash, show]

type 'ty possibly_enforced_ty = {
  et_enforced: enforcement;
  et_type: 'ty;
}
[@@deriving hash, show]

type 'ty fun_param = {
  fp_pos: Pos_or_decl.t;
  fp_name: string option;
  fp_type: 'ty possibly_enforced_ty;
  fp_flags: Typing_defs_flags.FunParam.t;
  fp_def_value: string option;
}
[@@deriving hash, show]

type 'ty fun_params = 'ty fun_param list [@@deriving hash]

(** The type of a function AND a method *)
type 'ty fun_type = {
  ft_tparams: 'ty tparam list;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_implicit_params: 'ty fun_implicit_params;
  ft_ret: 'ty possibly_enforced_ty;
  ft_flags: Typing_defs_flags.Fun.t;
  ft_cross_package: cross_package_decl;
}
[@@deriving hash, show]

(** = Reason.t * 'phase ty_ *)
type 'phase ty

and decl_ty = decl_phase ty

and locl_ty = locl_phase ty

(* A shape may specify whether or not fields are required. For example, consider
   this typedef:

     type ShapeWithOptionalField = shape(?'a' => ?int);

   With this definition, the field 'a' may be unprovided in a shape. In this
   case, the field 'a' would have sf_optional set to true.
*)
and 'phase shape_field_type = {
  sft_optional: bool;
  sft_ty: 'phase ty;
}

and _ ty_ =
  (*========== Following Types Exist Only in the Declared Phase ==========*)
  (* The late static bound type of a class *)
  | Tthis : decl_phase ty_
  (* Either an object type or a type alias, ty list are the arguments *)
  | Tapply : pos_id * decl_ty list -> decl_phase ty_
  | Trefinement : decl_ty * decl_phase class_refinement -> decl_phase ty_
  (* "Any" is the type of a variable with a missing annotation, and "mixed" is
   * the type of a variable annotated as "mixed". THESE TWO ARE VERY DIFFERENT!
   * Any unifies with anything, i.e., it is both a supertype and subtype of any
   * other type. You can do literally anything to it; it's the "trust me" type.
   * Mixed, on the other hand, is only a supertype of everything. You need to do
   * a case analysis to figure out what it is (i.e., its elimination form).
   *
   * Here's an example to demonstrate:
   *
   * function f($x): int {
   *   return $x + 1;
   * }
   *
   * In that example, $x has type Tany. This unifies with anything, so adding
   * one to it is allowed, and returning that as int is allowed.
   *
   * In contrast, if $x were annotated as mixed, adding one to that would be
   * a type error -- mixed is not a subtype of int, and you must be a subtype
   * of int to take part in addition. (The converse is true though -- int is a
   * subtype of mixed.) A case analysis would need to be done on $x, via
   * is_int or similar.
   *
   * mixed exists only in the decl_phase phase because it is desugared into ?nonnull
   * during the localization phase.
   *)
  | Tmixed : decl_phase ty_
  | Twildcard : decl_phase ty_
      (** Various intepretations, depending on context.
        *   inferred type e.g. (vec<_> $x) ==> $x[0]
        *   placeholder in refinement e.g. $x as Vector<_>
        *   placeholder for higher-kinded formal type parameter e.g. foo<T1<_>>(T1<int> $_)
        *)
  | Tlike : decl_ty -> decl_phase ty_
  (*========== Following Types Exist in Both Phases ==========*)
  | Tany : TanySentinel.t -> 'phase ty_
  | Tnonnull
  (* A dynamic type is a special type which sometimes behaves as if it were a
   * top type; roughly speaking, where a specific value of a particular type is
   * expected and that type is dynamic, anything can be given. We call this
   * behaviour "coercion", in that the types "coerce" to dynamic. In other ways it
   * behaves like a bottom type; it can be used in any sort of binary expression
   * or even have object methods called from it. However, it is in fact neither.
   *
   * it captures dynamicism within function scope.
   * See tests in typecheck/dynamic/ for more examples.
   *)
  | Tdynamic
  (* Nullable, called "option" in the ML parlance. *)
  | Toption : 'phase ty -> 'phase ty_
  (* All the primitive types: int, string, void, etc. *)
  | Tprim : Aast.tprim -> 'phase ty_
  (* A wrapper around fun_type, which contains the full type information for a
   * function, method, lambda, etc. *)
  | Tfun : 'phase ty fun_type -> 'phase ty_
  (* Tuple, with ordered list of the types of the elements of the tuple. *)
  | Ttuple : 'phase ty list -> 'phase ty_
  (* A wrapper around shape_type, which contains information about shape fields *)
  | Tshape : 'phase shape_type -> 'phase ty_
  (* The type of a generic parameter. The constraints on a generic parameter
   * are accessed through the lenv.tpenv component of the environment, which
   * is set up when checking the body of a function or method. See uses of
   * Typing_phase.add_generic_parameters_and_constraints. The list denotes
   * type arguments (for higher-kinded generics).
   *)
  | Tgeneric : string * 'phase ty list -> 'phase ty_
  (* Union type.
   * The values that are members of this type are the union of the values
   * that are members of the components of the union.
   * Some examples (writing | for binary union)
   *   Tunion []  is the "nothing" type, with no values
   *   Tunion [int;float] is the same as num
   *   Tunion [null;t] is the same as Toption t
   *)
  | Tunion : 'phase ty list -> 'phase ty_
  | Tintersection : 'phase ty list -> 'phase ty_
  (* Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>" *)
  | Tvec_or_dict : 'phase ty * 'phase ty -> 'phase ty_
  (* Name of class, name of type const, remaining names of type consts *)
  | Taccess : 'phase taccess_type -> 'phase ty_
  (* The type of an opaque type (e.g. a "newtype" outside of the file where it
   * was defined) or enum. They are "opaque", which means that they only unify with
   * themselves. However, it is possible to have a constraint that allows us to
   * relax this. For example:
   *
   *   newtype my_type as int = ...
   *
   * Outside of the file where the type was defined, this translates to:
   *
   *   Tnewtype ((pos, "my_type"), [], Tprim Tint)
   *
   * Which means that my_type is abstract, but is subtype of int as well.
   *)
  | Tnewtype : string * 'phase ty list * 'phase ty -> 'phase ty_
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)
  | Tvar : Tvid.t -> locl_phase ty_
  (* This represents a type alias that lacks necessary type arguments. Given
   *   type Foo<T1,T2> = ...
   * Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
   * further type arguments. In particular, Tunappliedalias always stands for
   * a higher-kinded type. It is never used for an alias like
   *   type Foo2 = ...
   * that simply doesn't require type arguments.
   *)
  | Tunapplied_alias : string -> locl_phase ty_
  (* see dependent_type *)
  | Tdependent : dependent_type * locl_ty -> locl_phase ty_
  (* An instance of a class or interface, ty list are the arguments
   * If exact=Exact, then this represents instances of *exactly* this class
   * If exact=Nonexact, this also includes subclasses
   *)
  | Tclass : pos_id * exact * locl_ty list -> locl_phase ty_
  | Tneg : neg_type -> locl_phase ty_

and exact =
  | Exact
  | Nonexact of locl_phase class_refinement

and 'phase class_refinement = { cr_consts: 'phase refined_const SMap.t }

and 'phase refined_const = {
  rc_bound: 'phase refined_const_bound;
  rc_is_ctx: bool;
}

and 'phase refined_const_bound =
  | TRexact : 'phase ty -> 'phase refined_const_bound
  | TRloose : 'phase refined_const_bounds -> 'phase refined_const_bound

and 'phase refined_const_bounds = {
  tr_lower: 'phase ty list;
  tr_upper: 'phase ty list;
}

and 'phase taccess_type = 'phase ty * pos_id

(* Whether all fields of this shape are known, types of each of the
 * known arms. *)
and 'phase shape_type = {
  s_origin: type_origin;
  s_unknown_value: 'phase ty;
  s_fields: 'phase shape_field_type TShapeMap.t;
}
[@@deriving hash]

val equal_decl_ty : decl_ty -> decl_ty -> bool

val ty_compare : ?normalize_lists:bool -> 'ph ty -> 'ph ty -> int

val ty__compare : ?normalize_lists:bool -> 'ph ty_ -> 'ph ty_ -> int

val tyl_compare :
  sort:bool -> ?normalize_lists:bool -> 'ph ty list -> 'ph ty list -> int

val equal_shape_field_type :
  decl_phase shape_field_type -> decl_phase shape_field_type -> bool

(** Whether equal modulo reason and position info. *)
val exact_equal : exact -> exact -> bool

val nonexact : exact

val is_nonexact : exact -> bool

val refined_const_kind_str : 'phase refined_const -> string

module Flags : sig
  val get_ft_return_disposable : 'a fun_type -> bool

  val get_ft_returns_readonly : 'a fun_type -> bool

  val set_ft_returns_readonly : 'a fun_type -> bool -> 'a fun_type

  val get_ft_async : 'a fun_type -> bool

  val get_ft_generator : 'a fun_type -> bool

  val get_ft_ftk : 'a fun_type -> fun_tparams_kind

  val set_ft_ftk : 'a fun_type -> fun_tparams_kind -> 'a fun_type

  val set_ft_is_function_pointer : 'a fun_type -> bool -> 'a fun_type

  val get_ft_is_function_pointer : 'a fun_type -> bool

  val get_ft_fun_kind : 'a fun_type -> Ast_defs.fun_kind

  val get_ft_readonly_this : 'a fun_type -> bool

  val set_ft_readonly_this : 'a fun_type -> bool -> 'a fun_type

  val get_ft_support_dynamic_type : 'a fun_type -> bool

  val set_ft_support_dynamic_type : 'a fun_type -> bool -> 'a fun_type

  val get_ft_is_memoized : 'a fun_type -> bool

  val get_ft_variadic : 'a fun_type -> bool

  val get_fp_readonly : 'a fun_param -> bool

  val make_fp_flags :
    mode:param_mode ->
    accept_disposable:bool ->
    has_default:bool ->
    readonly:bool ->
    Typing_defs_flags.FunParam.t

  val get_fp_accept_disposable : 'a fun_param -> bool

  val get_fp_has_default : 'a fun_param -> bool

  val get_fp_mode : 'a fun_param -> param_mode
end

include module type of Flags

module Pp : sig
  val pp_ty : Format.formatter -> 'a ty -> unit

  val pp_decl_ty : Format.formatter -> decl_ty -> unit

  val pp_locl_ty : Format.formatter -> locl_ty -> unit

  val pp_ty_ : Format.formatter -> 'a ty_ -> unit

  val pp_taccess_type : Format.formatter -> 'a taccess_type -> unit

  val pp_shape_field_type : Format.formatter -> 'a shape_field_type -> unit

  val show_decl_ty : decl_ty -> string

  val show_locl_ty : locl_ty -> string
end

include module type of Pp

type decl_ty_ = decl_phase ty_

type locl_ty_ = locl_phase ty_

type decl_tparam = decl_ty tparam [@@deriving eq, show]

type locl_tparam = locl_ty tparam

type decl_where_constraint = decl_ty where_constraint [@@deriving eq, show]

type locl_where_constraint = locl_ty where_constraint

type decl_fun_type = decl_ty fun_type [@@deriving eq]

type locl_fun_type = locl_ty fun_type

type decl_possibly_enforced_ty = decl_ty possibly_enforced_ty [@@deriving eq]

type locl_possibly_enforced_ty = locl_ty possibly_enforced_ty [@@deriving show]

type decl_fun_param = decl_ty fun_param [@@deriving eq]

type locl_fun_param = locl_ty fun_param

type decl_fun_params = decl_ty fun_params [@@deriving eq]

type locl_fun_params = locl_ty fun_params

type decl_class_refinement = decl_phase class_refinement

type locl_class_refinement = locl_phase class_refinement

type decl_refined_const = decl_phase refined_const

type locl_refined_const = locl_phase refined_const

type has_member = {
  hm_name: Nast.sid;
  hm_type: locl_ty;
  hm_class_id: Nast.class_id_;
      (** This is required to check ambiguous object access, where sometimes
          HHVM would access the private member of a parent class instead of the
          one from the current class. *)
  hm_explicit_targs: Nast.targ list option;
      (* - For a "has-property" constraint, this is `None`
       * - For a "has-method" constraint, this is `Some targs`, where targs
       *   is the list of explicit type arguments provided to the method call.
       *   Note that this list can be empty (i.e. `Some []`) in the case of a
       *   method not taking type arguments, or when we leave them implicit
       *
       * We need to know if this is a "has-property" or "has-method" to pass
       * the correct `is_method` parameter to `Typing_object_get.obj_get`.
       *)
}
[@@deriving show]

type destructure_kind =
  | ListDestructure
  | SplatUnpack
[@@deriving eq, ord, show]

type destructure = {
  (* This represents the standard parameters of a function or the fields in a list
   * destructuring assignment. Example:
   *
   * function take(bool $b, float $f = 3.14, arraykey ...$aks): void {}
   * function f((bool, float, int, string) $tup): void {
   *   take(...$tup);
   * }
   *
   * corresponds to the subtyping assertion
   *
   * (bool, float, int, string) <: splat([#1], [opt#2], ...#3)
   *)
  d_required: locl_ty list;
  (* Represents the optional parameters in a function, only used for splats *)
  d_optional: locl_ty list;
  (* Represents a function's variadic parameter, also only used for splats *)
  d_variadic: locl_ty option;
  (* list() destructuring allows for partial matches on lists, even when the operation
   * might throw i.e. list($a) = vec[]; *)
  d_kind: destructure_kind;
}
[@@deriving show]

(* A can_index constraint represents the ability to do an array index operation.
 * We should have t <: { ci_key; ci_shape; ci_val } when t is a type that supports
 * being index with a value of type ci_key, and will return a value of ci_val. The
 * ci_shape field is necessary because shapes (and tuples) are required to be
 * indexed with certain literals, and so in those cases it is not sufficient to
 * record simply the type of the index.
 *)
type can_index = {
  ci_key: locl_ty;
  ci_shape: tshape_field_name option;
  ci_val: locl_ty;
  ci_expr_pos: Pos.t;
  ci_index_pos: Pos.t;
}
[@@deriving show]

(* A can_traverse represents the ability to do a foreach over a certain type.
   We should have t <: {ct_key; ct_val; ct_is_await} when type t supports foreach
   and doing the foreack will bind values of type ct_val, and optionally bind keys of
   type ct_key. *)
type can_traverse = {
  ct_key: locl_ty option;
  ct_val: locl_ty;
  ct_is_await: bool;
  ct_reason: Reason.t;
}
[@@deriving show]

type has_type_member = {
  htm_id: string;
  htm_lower: locl_ty;
  htm_upper: locl_ty;
}
[@@deriving show]

(* = Reason.t * constraint_type_ *)
type constraint_type [@@deriving show]

type constraint_type_ =
  | Thas_member of has_member
  | Thas_type_member of has_type_member
      (** [Thas_type_member('T',lo,hi)] is a supertype of all concrete class
          types that have a type member [::T] satisfying [lo <: T <: hi] *)
  | Tcan_index of can_index
  | Tcan_traverse of can_traverse
  | Tdestructure of destructure
      (** The type of a list destructuring assignment.
          Implements valid destructuring operations via subtyping. *)
  | TCunion of locl_ty * constraint_type
  | TCintersection of locl_ty * constraint_type
[@@deriving show]

type internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type
[@@deriving eq, show]

(** Returns [true] if both origins are available and identical.
    If this function returns [true], the two types that have
    the origins provided must be identical. *)
val same_type_origin : type_origin -> type_origin -> bool

val ft_params_compare :
  ?normalize_lists:bool ->
  'a ty fun_param list ->
  'a ty fun_param list ->
  Ppx_deriving_runtime.int

val mk : 'phase Reason.t_ * 'phase ty_ -> 'phase ty

val deref : 'phase ty -> 'phase Reason.t_ * 'phase ty_

val get_reason : 'phase ty -> 'phase Reason.t_

val get_node : 'phase ty -> 'phase ty_

val with_reason : 'phase ty -> 'phase Reason.t_ -> 'phase ty

val get_pos : 'phase ty -> Pos_or_decl.t

val map_reason :
  'phase ty -> f:('phase Reason.t_ -> 'phase Reason.t_) -> 'phase ty

val map_ty : 'ph ty -> f:('ph ty_ -> 'ph ty_) -> 'ph ty

val mk_constraint_type : Reason.t * constraint_type_ -> constraint_type

val deref_constraint_type : constraint_type -> Reason.t * constraint_type_

val get_reason_i : internal_type -> Reason.t

(** Hack keyword for this visibility *)
val string_of_visibility : ce_visibility -> string

val compare_locl_ty : ?normalize_lists:bool -> locl_ty -> locl_ty -> int

val compare_decl_ty : ?normalize_lists:bool -> decl_ty -> decl_ty -> int

val tyl_equal : 'a ty list -> 'a ty list -> bool

val class_id_equal : ('a, 'b) Aast.class_id_ -> ('c, 'd) Aast.class_id_ -> bool

val constraint_ty_compare :
  ?normalize_lists:bool ->
  constraint_type ->
  constraint_type ->
  Ppx_deriving_runtime.int

val ty_equal : ?normalize_lists:bool -> 'a ty -> 'a ty -> bool

val equal_locl_ty : locl_ty -> locl_ty -> bool

val equal_locl_ty_ : locl_ty_ -> locl_ty_ -> bool

val equal_decl_tyl : decl_ty list -> decl_ty list -> bool
