(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Reason = Typing_reason
module SN = Naming_special_names

type visibility =
  | Vpublic
  | Vprivate of string
  | Vprotected of string
[@@deriving eq, show]

type exact =
  | Exact
  | Nonexact
[@@deriving eq, ord]

(* All the possible types, reason is a trace of why a type
   was inferred in a certain way.

   Types exists in two phases. Phase one is 'decl', meaning it is a type that
   was declared in user code. Phase two is 'locl', meaning it is a type that is
   inferred via local inference.
*)
(* create private types to represent the different type phases *)
type decl_phase = private DeclPhase [@@deriving eq]

type locl_phase = private LoclPhase [@@deriving eq]

type val_kind =
  | Lval
  | LvalSubexpr
  | Other
[@@deriving eq]

type param_mutability =
  | Param_owned_mutable
  | Param_borrowed_mutable
  | Param_maybe_mutable
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
[@@deriving eq, ord]

type param_mode =
  | FPnormal
  | FPinout
[@@deriving eq]

type xhp_attr_tag =
  | Required
  | Lateinit
[@@deriving eq]

type xhp_attr = {
  xa_tag: xhp_attr_tag option;
  xa_has_default: bool;
}
[@@deriving eq]

(** Denotes the categories of requirements we apply to constructor overrides.
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
[@@deriving eq]

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
  (* A class name, new type, or generic, i.e.
   *
   * abstract class C { abstract const type T }
   *
   * The type C::T is (`cls '\C', ['T'])
   *)
  | DTcls of string
  (* A reference to some expression. For example:
   *
   *  $x->foo()
   *
   *  The expression $x would have a reference Ident.t
   *  The expression $x->foo() would have a different one
   *)
  | DTexpr of Ident.t
[@@deriving eq]

type destructure_kind =
  | ListDestructure
  | SplatUnpack
[@@deriving eq, ord]

type 'ty tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: Ast_defs.id;
  tp_constraints: (Ast_defs.constraint_kind * 'ty) list;
  tp_reified: Aast.reify_kind;
  tp_user_attributes: Nast.user_attribute list;
}
[@@deriving eq]

type 'ty where_constraint = 'ty * Ast_defs.constraint_kind * 'ty [@@deriving eq]

type 'phase ty = Reason.t * 'phase ty_

and decl_ty = decl_phase ty

and locl_ty = locl_phase ty

and locl_ty_ = locl_phase ty_

(** A shape may specify whether or not fields are required. For example, consider
 * this typedef:
 *
 * ```
 * type ShapeWithOptionalField = shape(?'a' => ?int);
 * ```
 *
 * With this definition, the field 'a' may be unprovided in a shape. In this
 * case, the field 'a' would have sf_optional set to true.
 *)
and 'phase shape_field_type = {
  sft_optional: bool;
  sft_ty: 'phase ty;
}

and _ ty_ =
  (*========== Following Types Exist Only in the Declared Phase ==========*)
  | Tthis : decl_phase ty_  (** The late static bound type of a class *)
  | Tapply : Nast.sid * decl_ty list -> decl_phase ty_
      (** Either an object type or a type alias, ty list are the arguments *)
  | Taccess : taccess_type -> decl_phase ty_
      (** Name of class, name of type const, remaining names of type consts *)
  | Tarray : decl_ty option * decl_ty option -> decl_phase ty_
      (** The type of the various forms of "array":
       *
       * ```
       * Tarray (None, None)         => "array"
       * Tarray (Some ty, None)      => "array<ty>"
       * Tarray (Some ty1, Some ty2) => "array<ty1, ty2>"
       * Tarray (None, Some ty)      => [invalid]
       * ```
       *)
  | Tdarray : decl_ty * decl_ty -> decl_phase ty_
      (** Tdarray (ty1, ty2) => "darray<ty1, ty2>" *)
  | Tvarray : decl_ty -> decl_phase ty_  (** Tvarray (ty) => "varray<ty>" *)
  | Tvarray_or_darray : decl_ty option * decl_ty -> decl_phase ty_
      (** Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>" *)
  | Tmixed : decl_phase ty_
      (** "Any" is the type of a variable with a missing annotation, and "mixed" is
       * the type of a variable annotated as "mixed". THESE TWO ARE VERY DIFFERENT!
       * Any unifies with anything, i.e., it is both a supertype and subtype of any
       * other type. You can do literally anything to it; it's the "trust me" type.
       * Mixed, on the other hand, is only a supertype of everything. You need to do
       * a case analysis to figure out what it is (i.e., its elimination form).
       *
       * Here's an example to demonstrate:
       *
       * ```
       * function f($x): int {
       *   return $x + 1;
       * }
       * ```
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
  | Tlike : decl_ty -> decl_phase ty_
  | Tpu_access : decl_ty * Nast.sid * Nast.pu_loc -> decl_phase ty_
      (** Access to a Pocket Universe or Pocket Universes dependent type,
       * denoted by Foo:@Bar, or Foo:@Bar:@Bli:@T.
       * It might be unresolved at first (e.g. if Bli is a generic variable).
       * Will be refined to Tpu, or to the actual type associated with an
       * atom, once typechecking is successful.
       *)
  (*========== Following Types Exist in Both Phases ==========*)
  | Tany : TanySentinel.t -> 'phase ty_
  | Terr
  | Tnonnull
  | Tdynamic
      (** A dynamic type is a special type which sometimes behaves as if it were a
       * top type; roughly speaking, where a specific value of a particular type is
       * expected and that type is dynamic, anything can be given. We call this
       * behaviour "coercion", in that the types "coerce" to dynamic. In other ways it
       * behaves like a bottom type; it can be used in any sort of binary expression
       * or even have object methods called from it. However, it is in fact neither.
       *
       * it captures dynamicism within function scope.
       * See tests in typecheck/dynamic/ for more examples.
       *)
  | Toption : 'phase ty -> 'phase ty_
      (** Nullable, called "option" in the ML parlance. *)
  | Tprim : Aast.tprim -> 'phase ty_
      (** All the primitive types: int, string, void, etc. *)
  | Tfun : 'phase ty fun_type -> 'phase ty_
      (** A wrapper around fun_type, which contains the full type information for a
       * function, method, lambda, etc. *)
  | Ttuple : 'phase ty list -> 'phase ty_
      (** Tuple, with ordered list of the types of the elements of the tuple. *)
  | Tshape : shape_kind * 'phase shape_field_type Nast.ShapeMap.t -> 'phase ty_
      (** Whether all fields of this shape are known, types of each of the
       * known arms.
       *)
  | Tvar : Ident.t -> 'phase ty_
  | Tgeneric : string -> 'phase ty_
      (** The type of a generic parameter. The constraints on a generic parameter
       * are accessed through the lenv.tpenv component of the environment, which
       * is set up when checking the body of a function or method. See uses of
       * Typing_phase.localize_generic_parameters_with_bounds.
       *)
  | Tunion : 'phase ty list -> 'phase ty_
      (** Union type.
       * The values that are members of this type are the union of the values
       * that are members of the components of the union.
       * Some examples (writing | for binary union)
       *   Tunion []  is the "nothing" type, with no values
       *   Tunion [int;float] is the same as num
       *   Tunion [null;t] is the same as Toption t
       *)
  | Tintersection : 'phase ty list -> 'phase ty_
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)
  | Tnewtype : string * locl_ty list * locl_ty -> locl_phase ty_
      (** The type of an opaque type (e.g. a "newtype" outside of the file where it
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
  | Tdependent : dependent_type * locl_ty -> locl_phase ty_
      (** see dependent_type *)
  | Tobject : locl_phase ty_
      (** Tobject is an object type compatible with all objects. This type is also
       * compatible with some string operations (since a class might implement
       * __toString), but not with string type hints.
       *
       * Tobject is currently used to type code like:
       *   ../test/typecheck/return_unknown_class.php
       *)
  | Tclass : Nast.sid * exact * locl_ty list -> locl_phase ty_
      (** An instance of a class or interface, ty list are the arguments
       * If exact=Exact, then this represents instances of *exactly* this class
       * If exact=Nonexact, this also includes subclasses
       *)
  | Tarraykind : array_kind -> locl_phase ty_
      (** Localized version of Tarray *)
  | Tpu : locl_ty * Nast.sid -> locl_phase ty_
      (** Typing of Pocket Universe Expressions
       * - first parameter is the enclosing class
       * - second parameter is the name of the Pocket Universe Enumeration
       *)
  | Tpu_type_access : locl_ty * Nast.sid * Nast.sid * Nast.sid -> locl_phase ty_
      (** Typing of Pocket Universes type projections
       * - first parameter is the enclosing class
       * - second parameter is the name of the Pocket Universe Enumeration
       * - third parameter is the Tgeneric in place of the
       *   member name
       * - the fourth parameter is the name of the type to project
       *)

and constraint_type_ =
  | Thas_member of has_member
  | Tdestructure of destructure
      (** The type of container destructuring via list() or splat `...` *)
  | TCunion of locl_ty * constraint_type
  | TCintersection of locl_ty * constraint_type

and has_member = {
  hm_name: Nast.sid;
  hm_type: locl_ty;
  hm_class_id: Nast.class_id_;
      (** This is required to check ambiguous object access, where sometimes
  HHVM would access the private member of a parent class instead of the
  one from the current class. *)
}

and destructure = {
  d_required: locl_ty list;
      (** This represents the standard parameters of a function or the fields in a list
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
  d_optional: locl_ty list;
      (** Represents the optional parameters in a function, only used for splats *)
  d_variadic: locl_ty option;
      (** Represents a function's variadic parameter, also only used for splats *)
  d_kind: destructure_kind;
      (** list() destructuring allows for partial matches on lists, even when the operation
       * might throw i.e. list($a) = vec[]; *)
}

and constraint_type = Reason.t * constraint_type_

and internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type

and array_kind =
  | AKvarray of locl_ty  (** An array declared as a varray. *)
  | AKdarray of locl_ty * locl_ty  (** An array declared as a darray. *)
  | AKvarray_or_darray of locl_ty * locl_ty
      (** An array annotated as a varray_or_darray. *)

and taccess_type = decl_ty * Nast.sid list

(** represents reactivity of function
   - None corresponds to non-reactive function
   - Some reactivity - to reactive function with specified reactivity flavor

 Nonreactive <: Local -t <: Shallow -t <: Reactive -t

 MaybeReactive represents conditional reactivity of function that depends on
   reactivity of function arguments

```
   <<__Rx>>
   function f(<<__MaybeRx>> $g) { ... }
```

   call to function f will be treated as reactive only if $g is reactive
  *)
and reactivity =
  | Nonreactive
  | Local of decl_ty option
  | Shallow of decl_ty option
  | Reactive of decl_ty option
  | Pure of decl_ty option
  | MaybeReactive of reactivity
  | RxVar of reactivity option

(** The type of a function AND a method.
 * A function has a min and max arity because of optional arguments *)
and 'ty fun_type = {
  ft_arity: 'ty fun_arity;
  ft_tparams: 'ty tparam list;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_ret: 'ty possibly_enforced_ty;
      (** Carries through the sync/async information from the aast *)
  ft_reactive: reactivity;
  ft_flags: int;
}

and decl_fun_type = decl_ty fun_type

and locl_fun_type = locl_ty fun_type

(** Arity information for a fun_type; indicating the minimum number of
 * args expected by the function and the maximum number of args for
 * standard, non-variadic functions or the type of variadic argument taken *)
and 'ty fun_arity =
  | Fstandard of int  (** min; max is List.length ft_params *)
  | Fvariadic of int * 'ty fun_param
      (** PHP5.6-style ...$args finishes the func declaration.
          min ; variadic param type *)
  | Fellipsis of int * Pos.t
      (** HH-style ... anonymous variadic arg; body presumably uses func_get_args.
          min ; position of ... *)

and decl_fun_arity = decl_ty fun_arity

and locl_fun_arity = locl_ty fun_arity

and param_rx_annotation =
  | Param_rx_var
  | Param_rx_if_impl of decl_ty

and 'ty possibly_enforced_ty = {
  et_enforced: bool;
      (** True if consumer of this type enforces it at runtime *)
  et_type: 'ty;
}

and decl_possibly_enforced_ty = decl_ty possibly_enforced_ty

and locl_possibly_enforced_ty = locl_ty possibly_enforced_ty

and 'ty fun_param = {
  fp_pos: Pos.t;
  fp_name: string option;
  fp_type: 'ty possibly_enforced_ty;
  fp_kind: param_mode;
  fp_accept_disposable: bool;
  fp_mutability: param_mutability option;
  fp_rx_annotation: param_rx_annotation option;
}

and decl_fun_param = decl_ty fun_param

and locl_fun_param = locl_ty fun_param

and 'ty fun_params = 'ty fun_param list

and decl_fun_params = decl_ty fun_params

and locl_fun_params = locl_ty fun_params

(* [@@deriving ord] doesn't support GADT. If we want to get
 * rid of this one, we will have to write it *)
let compare_decl_ty : decl_ty -> decl_ty -> int = Stdlib.compare

let equal_locl_ty : locl_ty -> locl_ty -> bool = Stdlib.( = )

(* Constructor and deconstructor functions for types and constraint types.
 * Abstracting these lets us change the implementation, e.g. hash cons
 *)
let mk p = p

let deref p = p

let get_reason (r, _) = r

let get_node (_, n) = n

let get_pos t = Reason.to_pos (get_reason t)

let mk_constraint_type p = p

let deref_constraint_type p = p
