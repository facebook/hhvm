(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Reason = Typing_reason

type pos_id = Reason.pos_id [@@deriving eq, ord, show]

type ce_visibility =
  | Vpublic
  | Vprivate of string
  | Vprotected of string
[@@deriving eq, show]

(* Represents <<Policied()>> or <<InferFlows>> attribute *)
type ifc_fun_decl =
  | FDPolicied of string option
  | FDInferFlows
[@@deriving eq, ord]

val default_ifc_fun_decl : ifc_fun_decl

type exact =
  | Exact
  | Nonexact
[@@deriving eq, ord, show]

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

type shape_kind =
  | Closed_shape
  | Open_shape
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
  type t = tshape_field_name [@@deriving eq, ord]

  val pos : t -> Pos_or_decl.t

  val of_ast : (Pos.t -> Pos_or_decl.t) -> Ast_defs.shape_field_name -> t
end

(** This is similar to Aast.ShapeMap, but contains Pos_or_decl.t
    instead of Pos.t. Aast.ShapeMap is used in shape expressions,
    while this is used in shape types. *)
module TShapeMap : sig
  include WrappedMap.S with type key = TShapeField.t

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val map_and_rekey : 'a t -> (key -> key) -> ('a -> 'b) -> 'b t
end

module TShapeSet : Caml.Set.S with type elt = TShapeField.t

type param_mode =
  | FPnormal
  | FPinout
[@@deriving eq, show]

type xhp_attr_tag =
  | Required
  | Lateinit
[@@deriving eq, show]

type xhp_attr = {
  xa_tag: xhp_attr_tag option;
  xa_has_default: bool;
}
[@@deriving eq, show]

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

(* A dependent type consists of a base kind which indicates what the type is
 * dependent on. It is either dependent on:
 *  - The type 'this'
 *  - A class
 *  - An expression
 *
 * Dependent types also have a path component (derived from accessing a type
 * constant). Thus the dependent type (`expr 0, ['A', 'B', 'C']) roughly means
 * "The type resulting from accessing the type constant A then the type constant
 * B and then the type constant C on the expression reference by 0"
 *)
type dependent_type =
  (* Type that is the subtype of the late bound type within a class. *)
  | DTthis
  (* A reference to some expression. For example:
   *
   *  $x->foo()
   *
   *  The expression $x would have a reference Ident.t
   *  The expression $x->foo() would have a different one
   *)
  | DTexpr of Ident.t
[@@deriving eq, ord, show]

type user_attribute = {
  ua_name: pos_id;
  ua_classname_params: string list;
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
  (* The consumer doesn't enforce the type at runtime *)
  | Unenforced
  (* The consumer enforces the type at runtime *)
  | Enforced
  (* The consumer enforces part of the type at runtime, e.g.,
   * in C<int> it enforces that the value is a C, but does not enforce the
   * type argument. *)
  | PartiallyEnforced
[@@deriving eq, show, ord]

(* = Reason.t * 'phase ty_ *)
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
  | Tlike : decl_ty -> decl_phase ty_
  (*========== Following Types Exist in Both Phases ==========*)
  | Tany : TanySentinel.t -> 'phase ty_
  | Terr
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
  (* Whether all fields of this shape are known, types of each of the
   * known arms.
   *)
  | Tshape : shape_kind * 'phase shape_field_type TShapeMap.t -> 'phase ty_
  | Tvar : Ident.t -> 'phase ty_
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
  (* Tdarray (ty1, ty2) => "darray<ty1, ty2>" *)
  | Tdarray : 'phase ty * 'phase ty -> 'phase ty_
  (* Tvarray (ty) => "varray<ty>" *)
  | Tvarray : 'phase ty -> 'phase ty_
  (* Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>" *)
  | Tvarray_or_darray : 'phase ty * 'phase ty -> 'phase ty_
  (* Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>" *)
  | Tvec_or_dict : 'phase ty * 'phase ty -> 'phase ty_
  (* Name of class, name of type const, remaining names of type consts *)
  | Taccess : 'phase taccess_type -> 'phase ty_
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)
  (* This represents a type alias that lacks necessary type arguments. Given
   *   type Foo<T1,T2> = ...
   * Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
   * further type arguments. In particular, Tunappliedalias always stands for
   * a higher-kinded type. It is never used for an alias like
   *   type Foo2 = ...
   * that simply doesn't require type arguments.
   *)
  | Tunapplied_alias : string -> locl_phase ty_
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
  | Tnewtype : string * locl_ty list * locl_ty -> locl_phase ty_
  (* see dependent_type *)
  | Tdependent : dependent_type * locl_ty -> locl_phase ty_
  (* Tobject is an object type compatible with all objects. This type is also
   * compatible with some string operations (since a class might implement
   * __toString), but not with string type hints.
   *
   * Tobject is currently used to type code like:
   *   ../test/typecheck/return_unknown_class.php
   *)
  | Tobject : locl_phase ty_
  (* An instance of a class or interface, ty list are the arguments
   * If exact=Exact, then this represents instances of *exactly* this class
   * If exact=Nonexact, this also includes subclasses
   *)
  | Tclass : pos_id * exact * locl_ty list -> locl_phase ty_

and 'phase taccess_type = 'phase ty * pos_id

(* Because Tfun is currently used as both a decl and locl ty, without this,
 * the HH\Contexts\defaults alias must be stored in shared memory for a
 * decl Tfun record. We can eliminate this if the majority of usages end up
 * explicit or if we separate decl and locl Tfuns. *)
and 'ty capability =
  | CapDefaults of Pos_or_decl.t (* Should not be used for lambda inference *)
  | CapTy of 'ty

(** Companion to fun_params type, intended to consolidate checking of
 * implicit params for functions. *)
and 'ty fun_implicit_params = { capability: 'ty capability }

(* The type of a function AND a method.
 * A function has a min and max arity because of optional arguments *)
and 'ty fun_type = {
  ft_arity: 'ty fun_arity;
  ft_tparams: 'ty tparam list;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_implicit_params: 'ty fun_implicit_params;
  ft_ret: 'ty possibly_enforced_ty;
  (* Carries through the sync/async information from the aast *)
  ft_flags: int;
  ft_ifc_decl: ifc_fun_decl;
}

(* Arity information for a fun_type; indicating the minimum number of
 * args expected by the function and the maximum number of args for
 * standard, non-variadic functions or the type of variadic argument taken *)
and 'ty fun_arity =
  (* min; max is List.length ft_params *)
  | Fstandard
  (* PHP5.6-style ...$args finishes the func declaration.
     min ; variadic param type *)
  | Fvariadic of 'ty fun_param

and 'ty possibly_enforced_ty = {
  et_enforced: enforcement;
  et_type: 'ty;
}

and 'ty fun_param = {
  fp_pos: Pos_or_decl.t;
  fp_name: string option;
  fp_type: 'ty possibly_enforced_ty;
  fp_flags: int;
}

and 'ty fun_params = 'ty fun_param list

module Flags : sig
  val get_ft_return_disposable : 'a fun_type -> bool

  val get_ft_returns_readonly : 'a fun_type -> bool

  val get_ft_async : 'a fun_type -> bool

  val get_ft_generator : 'a fun_type -> bool

  val get_ft_ftk : 'a fun_type -> fun_tparams_kind

  val set_ft_ftk : 'a fun_type -> fun_tparams_kind -> 'a fun_type

  val set_ft_is_function_pointer : 'a fun_type -> bool -> 'a fun_type

  val get_ft_is_function_pointer : 'a fun_type -> bool

  val get_ft_fun_kind : 'a fun_type -> Ast_defs.fun_kind

  val get_ft_readonly_this : 'a fun_type -> bool

  val get_fp_ifc_can_call : 'a fun_param -> bool

  val get_fp_ifc_external : 'a fun_param -> bool

  val get_fp_is_atom : 'a fun_param -> bool

  val get_fp_readonly : 'a fun_param -> bool

  val fun_kind_to_flags : Ast_defs.fun_kind -> Hh_prelude.Int.t

  val make_ft_flags :
    Ast_defs.fun_kind ->
    return_disposable:bool ->
    returns_readonly:bool ->
    readonly_this:bool ->
    Hh_prelude.Int.t

  val mode_to_flags : param_mode -> int

  val make_fp_flags :
    mode:param_mode ->
    accept_disposable:bool ->
    has_default:bool ->
    ifc_external:bool ->
    ifc_can_call:bool ->
    is_atom:bool ->
    readonly:bool ->
    Hh_prelude.Int.t

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

  val pp_ty_list : Format.formatter -> 'a ty list -> unit

  val pp_taccess_type : Format.formatter -> 'a taccess_type -> unit

  val pp_possibly_enforced_ty :
    (Format.formatter -> 'a ty -> unit) ->
    Format.formatter ->
    'a ty possibly_enforced_ty ->
    unit

  val pp_fun_implicit_params :
    Format.formatter -> 'a ty fun_implicit_params -> unit

  val pp_shape_field_type : Format.formatter -> 'a shape_field_type -> unit

  val pp_fun_type : Format.formatter -> 'a ty fun_type -> unit

  val pp_fun_arity : Format.formatter -> 'a ty fun_arity -> unit

  val pp_fun_param : Format.formatter -> 'a ty fun_param -> unit

  val pp_fun_params : Format.formatter -> 'a ty fun_params -> unit

  val show_decl_ty : decl_ty -> string

  val show_locl_ty : locl_ty -> string

  val pp_ifc_fun_decl : Format.formatter -> ifc_fun_decl -> unit
end

include module type of Pp

type decl_ty_ = decl_phase ty_

type locl_ty_ = locl_phase ty_

type decl_tparam = decl_ty tparam [@@deriving show]

type locl_tparam = locl_ty tparam

type decl_where_constraint = decl_ty where_constraint [@@deriving show]

type locl_where_constraint = locl_ty where_constraint

type decl_fun_type = decl_ty fun_type

type locl_fun_type = locl_ty fun_type

type decl_fun_arity = decl_ty fun_arity

type locl_fun_arity = locl_ty fun_arity

type decl_possibly_enforced_ty = decl_ty possibly_enforced_ty

type locl_possibly_enforced_ty = locl_ty possibly_enforced_ty [@@deriving show]

type decl_fun_param = decl_ty fun_param

type locl_fun_param = locl_ty fun_param

type decl_fun_params = decl_ty fun_params

type locl_fun_params = locl_ty fun_params

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

(* = Reason.t * constraint_type_ *)
type constraint_type [@@deriving show]

type constraint_type_ =
  | Thas_member of has_member
  (* The type of a list destructuring assignment.
   * Implements valid destructuring operations via subtyping *)
  | Tdestructure of destructure
  | TCunion of locl_ty * constraint_type
  | TCintersection of locl_ty * constraint_type
[@@deriving show]

type internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type
[@@deriving show]

(* Abstraction *)
val compare_decl_ty : decl_ty -> decl_ty -> int

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
