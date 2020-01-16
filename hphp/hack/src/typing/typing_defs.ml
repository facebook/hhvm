(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
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

(* This abstract type allows us to guard the construction of Tany to the
 * make_tany function in this module. *)
module TanySentinel : sig
  type t [@@deriving eq]

  val value : t
end = struct
  type t = () [@@deriving eq]

  let value : t = ()
end

let show_phase_ty _ = "<phase_ty>"

let pp_phase_ty _ _ = Printf.printf "%s\n" "<phase_ty>"

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
  | Tapply : Nast.sid * decl_ty list -> decl_phase ty_
  (* Name of class, name of type const, remaining names of type consts *)
  | Taccess : taccess_type -> decl_phase ty_
  (* The type of the various forms of "array":
   * Tarray (None, None)         => "array"
   * Tarray (Some ty, None)      => "array<ty>"
   * Tarray (Some ty1, Some ty2) => "array<ty1, ty2>"
   * Tarray (None, Some ty)      => [invalid]
   *)
  | Tarray : decl_ty option * decl_ty option -> decl_phase ty_
  (* Tdarray (ty1, ty2) => "darray<ty1, ty2>" *)
  | Tdarray : decl_ty * decl_ty -> decl_phase ty_
  (* Tvarray (ty) => "varray<ty>" *)
  | Tvarray : decl_ty -> decl_phase ty_
  (* Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>" *)
  | Tvarray_or_darray : decl_ty option * decl_ty -> decl_phase ty_
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
  | Tnothing : decl_phase ty_
  | Tlike : decl_ty -> decl_phase ty_
  (* Access to a Pocket Universe Expression or Atom, denoted by
   * Foo:@Bar or Foo:@Bar:@X.
   * It might be unresolved at first (e.g. if Foo is a generic variable).
   * Will be refined to Tpu once typechecking is successful
   *)
  | Tpu_access : decl_ty * Nast.sid -> decl_phase ty_
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
   * function, method, lambda, etc. Note that lambdas have an additional layer
   * of indirection before you get to Tfun -- see Tanon below. *)
  | Tfun : 'phase ty fun_type -> 'phase ty_
  (* Tuple, with ordered list of the types of the elements of the tuple. *)
  | Ttuple : 'phase ty list -> 'phase ty_
  (* Whether all fields of this shape are known, types of each of the
   * known arms.
   *)
  | Tshape : shape_kind * 'phase shape_field_type Nast.ShapeMap.t -> 'phase ty_
  (* Access to a Pocket Universe Expression or Atom, denoted by
   * Foo:@Bar or Foo:@Bar:@X.
   * It might be unresolved at first (e.g. if Foo is a generic variable).
   * Will be refined to Tpu once typechecking is successful
   *)
  | Tvar : Ident.t -> 'phase ty_
  (* The type of a generic parameter. The constraints on a generic parameter
   * are accessed through the lenv.tpenv component of the environment, which
   * is set up when checking the body of a function or method. See uses of
   * Typing_phase.localize_generic_parameters_with_bounds.
   *)
  | Tgeneric : string -> 'phase ty_
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)

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
  (* An anonymous function, including the fun arity, and the identifier to
   * type the body of the function. (The actual closure is stored in
   * Typing_env.env.genv.anons) *)
  | Tanon : locl_fun_arity * Ident.t -> locl_phase ty_
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
  (* Tobject is an object type compatible with all objects. This type is also
   * compatible with some string operations (since a class might implement
   * __toString), but not with string type hints. In a similar way, Tobject
   * is compatible with some array operations (since a class might implement
   * ArrayAccess), but not with array type hints.
   *
   * Tobject is currently used to type code like:
   *   ../test/typecheck/return_unknown_class.php
   *)
  | Tobject : locl_phase ty_
  (* An instance of a class or interface, ty list are the arguments
   * If exact=Exact, then this represents instances of *exactly* this class
   * If exact=Nonexact, this also includes subclasses
   *)
  | Tclass : Nast.sid * exact * locl_ty list -> locl_phase ty_
  (* Localized version of Tarray *)
  | Tarraykind : array_kind -> locl_phase ty_
  (* Typing of Pocket Universe Expressions
   * - first parameter is the enclosing class
   * - second parameter is the name of the Pocket Universe Enumeration
   * - third parameter is  either Pu_plain (the enumeration as the set of
   *   all its atoms) or Pu_atom (a specific atom in the enumeration)
   *)
  | Tpu : locl_ty * Nast.sid -> locl_phase ty_
  (* Typing of Pocket Universes type projections
   * - first parameter is the enclosing class
   * - second parameter is the name of the Pocket Universe Enumeration
   * - third parameter is the generic (tvar/tabstract) in place of the
   *   member name
   * - the fourth parameter is the name of the type to project
   *)
  | Tpu_type_access : locl_ty * Nast.sid * locl_ty * Nast.sid -> locl_phase ty_

and constraint_type_ =
  | Thas_member of has_member
  (* The type of a list destructuring assignment.
   * Implements valid destructuring operations via subtyping *)
  | Tdestructure of locl_ty list
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

and constraint_type = Reason.t * constraint_type_

and internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type

and array_kind =
  (* An array declared as a varray. *)
  | AKvarray of locl_ty
  (* An array declared as a darray. *)
  | AKdarray of locl_ty * locl_ty
  (* An array annotated as a varray_or_darray. *)
  | AKvarray_or_darray of locl_ty * locl_ty
  (* This is a type created when we see array() literal *)
  | AKempty

and taccess_type = decl_ty * Nast.sid list

(* represents reactivity of function
   - None corresponds to non-reactive function
   - Some reactivity - to reactive function with specified reactivity flavor

 Nonreactive <: Local -t <: Shallow -t <: Reactive -t

 MaybeReactive represents conditional reactivity of function that depends on
   reactivity of function arguments
   <<__Rx>>
   function f(<<__MaybeRx>> $g) { ... }
   call to function f will be treated as reactive only if $g is reactive
  *)
and reactivity =
  | Nonreactive
  | Local of decl_ty option
  | Shallow of decl_ty option
  | Reactive of decl_ty option
  | MaybeReactive of reactivity
  | RxVar of reactivity option

(* The type of a function AND a method.
 * A function has a min and max arity because of optional arguments *)
and 'ty fun_type = {
  ft_is_coroutine: bool;
  ft_arity: 'ty fun_arity;
  ft_tparams: 'ty tparam list * fun_tparams_kind;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_ret: 'ty possibly_enforced_ty;
  (* Carries through the sync/async information from the aast *)
  ft_fun_kind: Ast_defs.fun_kind;
  ft_reactive: reactivity;
  ft_return_disposable: bool;
  (* mutability of the receiver *)
  ft_mutability: param_mutability option;
  ft_returns_mutable: bool;
  ft_returns_void_to_rx: bool;
}

and decl_fun_type = decl_ty fun_type

and locl_fun_type = locl_ty fun_type

(* Arity information for a fun_type; indicating the minimum number of
 * args expected by the function and the maximum number of args for
 * standard, non-variadic functions or the type of variadic argument taken *)
and 'ty fun_arity =
  (* min ; max *)
  | Fstandard of int * int
  (* PHP5.6-style ...$args finishes the func declaration.
     min ; variadic param type *)
  | Fvariadic of int * 'ty fun_param
  (* HH-style ... anonymous variadic arg; body presumably uses func_get_args.
     min ; position of ... *)
  | Fellipsis of int * Pos.t

and decl_fun_arity = decl_ty fun_arity

and locl_fun_arity = locl_ty fun_arity

and param_rx_annotation =
  | Param_rx_var
  | Param_rx_if_impl of decl_ty

and 'ty possibly_enforced_ty = {
  (* True if consumer of this type enforces it at runtime *)
  et_enforced: bool;
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

and class_elt = {
  ce_abstract: bool;
  ce_final: bool;
  ce_xhp_attr: xhp_attr option;
  (* This field has different meanings in shallow mode and eager mode:
   * In shallow mode, true if this method has attribute __Override.
   * In eager mode, true if this method is originally defined in a trait,
   * AND has the override attribute, AND the trait does not inherit any
   * other method of that name. *)
  ce_override: bool;
  (* true if this static property has attribute __LSB *)
  ce_lsb: bool;
  (* true if this method has attribute __MemoizeLSB *)
  ce_memoizelsb: bool;
  (* true if this elt arose from require-extends or other mechanisms
     of hack "synthesizing" methods that were not written by the
     programmer. The eventual purpose of this is to make sure that
     elts that *are* written by the programmer take precedence over
     synthesized elts. *)
  ce_synthesized: bool;
  ce_visibility: visibility;
  ce_const: bool;
  ce_lateinit: bool;
  ce_type: decl_ty Lazy.t;
  (* identifies the class from which this elt originates *)
  ce_origin: string;
  ce_deprecated: string option;
  ce_pos: Pos.t Lazy.t;
}

and fun_elt = {
  fe_deprecated: string option;
  fe_type: decl_ty;
  fe_decl_errors: Errors.t option;
  fe_pos: Pos.t;
}

and class_const = {
  cc_synthesized: bool;
  cc_abstract: bool;
  cc_pos: Pos.t;
  cc_type: decl_ty;
  cc_expr: Nast.expr option;
  (* identifies the class from which this const originates *)
  cc_origin: string;
}

(* The position is that of the hint in the `use` / `implements` AST node
 * that causes a class to have this requirement applied to it. E.g.
 *
 * class Foo {}
 *
 * interface Bar {
 *   require extends Foo; <- position of the decl_phase ty
 * }
 *
 * class Baz extends Foo implements Bar { <- position of the `implements`
 * }
 *)
and requirement = Pos.t * decl_ty

and class_type = {
  tc_need_init: bool;
  (* Whether the typechecker knows of all (non-interface) ancestors
   * and thus known all accessible members of this class *)
  tc_members_fully_known: bool;
  tc_abstract: bool;
  tc_final: bool;
  tc_const: bool;
  (* True when the class is annotated with the __PPL attribute. *)
  tc_ppl: bool;
  (* When a class is abstract (or in a trait) the initialization of
   * a protected member can be delayed *)
  tc_deferred_init_members: SSet.t;
  tc_kind: Ast_defs.class_kind;
  tc_is_xhp: bool;
  tc_has_xhp_keyword: bool;
  tc_is_disposable: bool;
  tc_name: string;
  tc_pos: Pos.t;
  tc_tparams: decl_tparam list;
  tc_where_constraints: decl_where_constraint list;
  tc_consts: class_const SMap.t;
  tc_typeconsts: typeconst_type SMap.t;
  tc_pu_enums: pu_enum_type SMap.t;
  tc_props: class_elt SMap.t;
  tc_sprops: class_elt SMap.t;
  tc_methods: class_elt SMap.t;
  tc_smethods: class_elt SMap.t;
  (* the bool represents final constructor or __ConsistentConstruct *)
  tc_construct: class_elt option * consistent_kind;
  (* This includes all the classes, interfaces and traits this class is
   * using. *)
  tc_ancestors: decl_ty SMap.t;
  tc_req_ancestors: requirement list;
  tc_req_ancestors_extends: SSet.t;
  (* the extends of req_ancestors *)
  tc_extends: SSet.t;
  tc_enum_type: enum_type option;
  tc_sealed_whitelist: SSet.t option;
  tc_decl_errors: Errors.t option;
}

and typeconst_abstract_kind =
  | TCAbstract of decl_ty option
  | TCPartiallyAbstract
  | TCConcrete

and typeconst_type = {
  ttc_abstract: typeconst_abstract_kind;
  ttc_name: Nast.sid;
  ttc_constraint: decl_ty option;
  ttc_type: decl_ty option;
  ttc_origin: string;
  ttc_enforceable: Pos.t * bool;
  ttc_reifiable: Pos.t option;
}

and pu_enum_type = {
  tpu_name: Nast.sid;
  tpu_is_final: bool;
  tpu_case_types: (Nast.sid * Aast.reify_kind) SMap.t;
  tpu_case_values: (Nast.sid * decl_ty) SMap.t;
  tpu_members: pu_member_type SMap.t;
}

and pu_member_type = {
  tpum_atom: Nast.sid;
  tpum_types: (Nast.sid * decl_ty) SMap.t;
  tpum_exprs: Nast.sid SMap.t;
}

and enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
}

and record_field_req =
  | ValueRequired
  | HasDefaultValue

and record_def_type = {
  rdt_name: Nast.sid;
  rdt_extends: Nast.sid option;
  rdt_fields: (Nast.sid * record_field_req) list;
  rdt_abstract: bool;
  rdt_pos: Pos.t;
  rdt_errors: Errors.t option;
}

and typedef_type = {
  td_pos: Pos.t;
  td_vis: Aast.typedef_visibility;
  td_tparams: decl_tparam list;
  td_constraint: decl_ty option;
  td_type: decl_ty;
  td_decl_errors: Errors.t option;
}

and decl_tparam = decl_ty tparam

and locl_tparam = locl_ty tparam

and decl_where_constraint = decl_ty where_constraint

and locl_where_constraint = locl_ty where_constraint

type phase_ty =
  | DeclTy of decl_ty
  | LoclTy of locl_ty

type deserialization_error =
  (* The type was valid, but some component thereof was a decl_ty when we
  expected a locl_phase ty, or vice versa. *)
  | Wrong_phase of string
  (* The specific type or some component thereof is not one that we support
  deserializing, usually because not enough information was serialized to be
  able to deserialize it again. For example, lambda types (`Tanon`) contain a
  reference to an identifier (`Ident.t`), which is not serialized. *)
  | Not_supported of string
  (* The input JSON was invalid for some reason. *)
  | Deserialization_error of string

(* Tracks information about how a type was expanded *)
type expand_env = {
  (* A list of the type defs and type access we have expanded thus far. Used
   * to prevent entering into a cycle when expanding these types
   *)
  type_expansions: (Pos.t * string) list;
  substs: locl_ty SMap.t;
  this_ty: locl_ty;
  (* The class that the type is extracted from. Used for creating expression
   * dependent types for type constants.
   *)
  from_class: Nast.class_id_ option;
  quiet: bool;
  on_error: Errors.typing_error_callback;
      (** If what we are localizing or expanding comes from the decl heap for
  example, then some errors must be silenced since they must have already been
  raised when first typechecking whatever we have fetched from the heap.
  Setting {!quiet} to true will silence those errors.
  T54121530 aims at offering a better mechanism. *)
}

let is_locl_type = function
  | LoclType _ -> true
  | _ -> false

let has_expanded { type_expansions; _ } x =
  List.exists type_expansions (function
      | (_, x') when String.equal x x' -> true
      | _ -> false)

let reason = function
  | LoclType (r, _)
  | ConstraintType (r, _) ->
    r

let is_constraint_type = function
  | ConstraintType _ -> true
  | LoclType _ -> false

let is_union_or_inter_type ((_, ty) : locl_ty) =
  (* do not expand type here! *)
  match ty with
  | Toption _
  | Tunion _
  | Tintersection _ ->
    true
  | Terr
  | Tnonnull
  | Tdynamic
  | Tobject
  | Tany _
  | Tprim _
  | Tfun _
  | Ttuple _
  | Tshape _
  | Tpu_type_access _
  | Tpu _
  | Tvar _
  | Tnewtype _
  | Tdependent _
  | Tgeneric _
  | Tanon _
  | Tclass _
  | Tarraykind _ ->
    false

(* The identifier for this *)
let this = Local_id.make_scoped "$this"

(* This should be the ONLY way that Tany is constructed anywhere in the
 * codebase. *)
let make_tany () = Tany TanySentinel.value

let arity_min ft_arity : int =
  match ft_arity with
  | Fstandard (min, _)
  | Fvariadic (min, _)
  | Fellipsis (min, _) ->
    min

let get_param_mode callconv =
  match callconv with
  | Some Ast_defs.Pinout -> FPinout
  | None -> FPnormal

module DependentKind = struct
  let to_string = function
    | DTthis -> SN.Typehints.this
    | DTcls c -> c
    | DTexpr i ->
      let display_id = Reason.get_expr_display_id i in
      "<expr#" ^ string_of_int display_id ^ ">"

  let is_generic_dep_ty s = String_utils.is_substring "::" s
end

module ShapeFieldMap = struct
  include Nast.ShapeMap

  let map_and_rekey shape_map key_f value_f =
    let f_over_shape_field_type ({ sft_ty; _ } as shape_field_type) =
      { shape_field_type with sft_ty = value_f sft_ty }
    in
    Nast.ShapeMap.map_and_rekey shape_map key_f f_over_shape_field_type

  let map_env f env shape_map =
    let f_over_shape_field_type env _key ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    Nast.ShapeMap.map_env f_over_shape_field_type env shape_map

  let map f shape_map = map_and_rekey shape_map (fun x -> x) f

  let iter f shape_map =
    let f_over_shape_field_type shape_map_key { sft_ty; _ } =
      f shape_map_key sft_ty
    in
    Nast.ShapeMap.iter f_over_shape_field_type shape_map

  let iter_values f = iter (fun _ -> f)
end

module ShapeFieldList = struct
  include Hh_core.List

  let map_env env xs ~f =
    let f_over_shape_field_type env ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    Hh_core.List.map_env env xs ~f:f_over_shape_field_type
end

(*****************************************************************************)
(* Suggest mode *)
(*****************************************************************************)

(* Set to true when we are trying to infer the missing type hints. *)
let is_suggest_mode = ref false

(* Ordinal value for type constructor, for localized types *)
let ty_con_ordinal ty_ =
  match ty_ with
  | Tany _
  | Terr ->
    0
  | Toption (_, Tnonnull) -> 1
  | Tnonnull -> 2
  | Tdynamic -> 3
  | Toption _ -> 4
  | Tprim _ -> 5
  | Tfun _ -> 6
  | Ttuple _ -> 7
  | Tshape _ -> 8
  | Tvar _ -> 9
  | Tnewtype _ -> 10
  | Tgeneric _ -> 11
  | Tdependent _ -> 12
  | Tanon _ -> 13
  | Tunion _ -> 14
  | Tintersection _ -> 15
  | Tobject -> 16
  | Tclass _ -> 17
  | Tarraykind _ -> 18
  | Tpu _ -> 19
  | Tpu_type_access _ -> 20

(* Ordinal value for type constructor, for decl types *)
let decl_ty_con_ordinal ty_ =
  match ty_ with
  | Tany _
  | Terr ->
    0
  | Tthis -> 1
  | Tapply _ -> 2
  | Tgeneric _ -> 3
  | Taccess _ -> 4
  | Tarray _ -> 5
  | Tdarray _ -> 6
  | Tvarray _ -> 7
  | Tvarray_or_darray _ -> 8
  | Tmixed -> 9
  | Tnothing -> 10
  | Tlike _ -> 11
  | Tnonnull -> 12
  | Tdynamic -> 13
  | Toption _ -> 14
  | Tprim _ -> 15
  | Tfun _ -> 16
  | Ttuple _ -> 17
  | Tshape _ -> 18
  | Tpu_access _ -> 19
  | Tvar _ -> 20
  | Tunion _ -> 21
  | Tintersection _ -> 22

let array_kind_con_ordinal ak =
  match ak with
  | AKvarray _ -> 1
  | AKdarray _ -> 3
  | AKvarray_or_darray _ -> 4
  | AKempty -> 6

(* Compare two types syntactically, ignoring reason information and other
 * small differences that do not affect type inference behaviour. This
 * comparison function can be used to construct tree-based sets of types,
 * or to compare two types for "exact" equality.
 * Note that this function does *not* expand type variables, or type
 * aliases.
 * But if ty_compare ty1 ty2 = 0, then the types must not be distinguishable
 * by any typing rules.
 *
 * TODO(T52611361): Make this comparison exhaustive on ty1 to remove the _ catchall
 *)
let rec ty__compare ?(normalize_lists = false) ty_1 ty_2 =
  let rec ty__compare ty_1 ty_2 =
    match (ty_1, ty_2) with
    | (Tprim ty1, Tprim ty2) -> compare ty1 ty2
    | (Toption ty, Toption ty2) -> ty_compare ty ty2
    | (Tfun fty, Tfun fty2) -> tfun_compare fty fty2
    | (Tunion tyl1, Tunion tyl2)
    | (Tintersection tyl1, Tintersection tyl2)
    | (Ttuple tyl1, Ttuple tyl2) ->
      tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
    | (Tgeneric n1, Tgeneric n2) -> String.compare n1 n2
    | (Tnewtype (id, tyl, cstr1), Tnewtype (id2, tyl2, cstr2)) ->
      begin
        match String.compare id id2 with
        | 0 ->
          (match tyl_compare ~sort:false tyl tyl2 with
          | 0 -> ty_compare cstr1 cstr2
          | n -> n)
        | n -> n
      end
    | (Tdependent (d1, cstr1), Tdependent (d2, cstr2)) ->
      begin
        match compare d1 d2 with
        | 0 -> ty_compare cstr1 cstr2
        | n -> n
      end
    (* An instance of a class or interface, ty list are the arguments *)
    | (Tclass (id, exact, tyl), Tclass (id2, exact2, tyl2)) ->
      begin
        match String.compare (snd id) (snd id2) with
        | 0 ->
          begin
            match tyl_compare ~sort:false tyl tyl2 with
            | 0 -> compare_exact exact exact2
            | n -> n
          end
        | n -> n
      end
    | (Tarraykind ak1, Tarraykind ak2) -> array_kind_compare ak1 ak2
    | (Tshape (shape_kind1, fields1), Tshape (shape_kind2, fields2)) ->
      begin
        match compare_shape_kind shape_kind1 shape_kind2 with
        | 0 ->
          List.compare
            (fun (k1, v1) (k2, v2) ->
              match Ast_defs.ShapeField.compare k1 k2 with
              | 0 -> shape_field_type_compare v1 v2
              | n -> n)
            (Nast.ShapeMap.elements fields1)
            (Nast.ShapeMap.elements fields2)
        | n -> n
      end
    | (Tvar v1, Tvar v2) -> compare v1 v2
    | (Tanon (_, id1), Tanon (_, id2)) -> compare id1 id2
    | _ -> ty_con_ordinal ty_1 - ty_con_ordinal ty_2
  and shape_field_type_compare sft1 sft2 =
    match ty_compare sft1.sft_ty sft2.sft_ty with
    | 0 -> compare sft1.sft_optional sft2.sft_optional
    | n -> n
  and tfun_compare fty1 fty2 =
    match possibly_enforced_ty_compare fty1.ft_ret fty2.ft_ret with
    | 0 ->
      begin
        match ft_params_compare fty1.ft_params fty2.ft_params with
        | 0 ->
          compare
            ( fty1.ft_is_coroutine,
              fty1.ft_arity,
              fty1.ft_reactive,
              fty1.ft_return_disposable,
              fty1.ft_mutability,
              fty1.ft_returns_mutable )
            ( fty2.ft_is_coroutine,
              fty2.ft_arity,
              fty2.ft_reactive,
              fty2.ft_return_disposable,
              fty2.ft_mutability,
              fty2.ft_returns_mutable )
        | n -> n
      end
    | n -> n
  and array_kind_compare ak1 ak2 =
    match (ak1, ak2) with
    | (AKdarray (ty1, ty2), AKdarray (ty3, ty4))
    | (AKvarray_or_darray (ty1, ty2), AKvarray_or_darray (ty3, ty4)) ->
      tyl_compare ~sort:false [ty1; ty2] [ty3; ty4]
    | (AKvarray ty1, AKvarray ty2) -> ty_compare ty1 ty2
    | _ -> array_kind_con_ordinal ak1 - array_kind_con_ordinal ak2
  and ty_compare ty1 ty2 = ty__compare (snd ty1) (snd ty2) in
  ty__compare ty_1 ty_2

and ty_compare ?(normalize_lists = false) ty1 ty2 =
  ty__compare ~normalize_lists (snd ty1) (snd ty2)

and tyl_compare ~sort ?(normalize_lists = false) tyl1 tyl2 =
  let (tyl1, tyl2) =
    if sort then
      (List.sort ty_compare tyl1, List.sort ty_compare tyl2)
    else
      (tyl1, tyl2)
  in
  List.compare (ty_compare ~normalize_lists) tyl1 tyl2

and possibly_enforced_ty_compare ?(normalize_lists = false) ety1 ety2 =
  match ty_compare ~normalize_lists ety1.et_type ety2.et_type with
  | 0 -> Bool.compare ety1.et_enforced ety2.et_enforced
  | n -> n

and ft_params_compare ?(normalize_lists = false) params1 params2 =
  let rec ft_params_compare params1 params2 =
    List.compare ft_param_compare params1 params2
  and ft_param_compare param1 param2 =
    match
      possibly_enforced_ty_compare
        ~normalize_lists
        param1.fp_type
        param2.fp_type
    with
    | 0 ->
      compare
        (param1.fp_kind, param1.fp_accept_disposable, param1.fp_mutability)
        (param2.fp_kind, param2.fp_accept_disposable, param2.fp_mutability)
    | n -> n
  in
  ft_params_compare params1 params2

let tyl_equal tyl1 tyl2 = Int.equal 0 @@ tyl_compare ~sort:false tyl1 tyl2

let class_id_con_ordinal cid =
  match cid with
  | Aast.CIparent -> 0
  | Aast.CIself -> 1
  | Aast.CIstatic -> 2
  | Aast.CIexpr _ -> 3
  | Aast.CI _ -> 4

let class_id_compare cid1 cid2 =
  match (cid1, cid2) with
  | (Aast.CIexpr _e1, Aast.CIexpr _e2) -> 0
  | (Aast.CI (_, id1), Aast.CI (_, id2)) -> String.compare id1 id2
  | _ -> class_id_con_ordinal cid2 - class_id_con_ordinal cid1

let class_id_equal cid1 cid2 = Int.equal (class_id_compare cid1 cid2) 0

let has_member_compare ~normalize_lists hm1 hm2 =
  let ty_compare = ty_compare ~normalize_lists in
  let { hm_name = (_, m1); hm_type = ty1; hm_class_id = cid1 } = hm1 in
  let { hm_name = (_, m2); hm_type = ty2; hm_class_id = cid2 } = hm2 in
  match compare m1 m2 with
  | 0 ->
    (match ty_compare ty1 ty2 with
    | 0 -> class_id_compare cid1 cid2
    | comp -> comp)
  | comp -> comp

let constraint_ty_con_ordinal cty =
  match cty with
  | Thas_member _ -> 0
  | Tdestructure _ -> 1
  | TCunion _ -> 2
  | TCintersection _ -> 3

let rec constraint_ty_compare ?(normalize_lists = false) (_, ty1) (_, ty2) =
  match (ty1, ty2) with
  | (Thas_member hm1, Thas_member hm2) ->
    has_member_compare ~normalize_lists hm1 hm2
  | (Tdestructure tyl1, Tdestructure tyl2) -> tyl_compare ~sort:false tyl1 tyl2
  | (TCunion (lty1, cty1), TCunion (lty2, cty2))
  | (TCintersection (lty1, cty1), TCintersection (lty2, cty2)) ->
    let comp1 = ty_compare ~normalize_lists lty1 lty2 in
    if not @@ Int.equal comp1 0 then
      comp1
    else
      constraint_ty_compare ~normalize_lists cty1 cty2
  | (_, (Thas_member _ | Tdestructure _ | TCunion _ | TCintersection _)) ->
    constraint_ty_con_ordinal ty2 - constraint_ty_con_ordinal ty1

let constraint_ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal (constraint_ty_compare ~normalize_lists ty1 ty2) 0

let ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal 0 (ty_compare ~normalize_lists ty1 ty2)

let equal_internal_type ty1 ty2 =
  match (ty1, ty2) with
  | (LoclType ty1, LoclType ty2) -> ty_equal ~normalize_lists:true ty1 ty2
  | (ConstraintType ty1, ConstraintType ty2) ->
    constraint_ty_equal ~normalize_lists:true ty1 ty2
  | (_, (LoclType _ | ConstraintType _)) -> false

let equal_locl_ty ty1 ty2 = ty_equal ty1 ty2

let equal_locl_ty_ ty_1 ty_2 = Int.equal 0 (ty__compare ty_1 ty_2)

let equal_locl_fun_arity a1 a2 =
  match (a1, a2) with
  | (Fstandard (min1, max1), Fstandard (min2, max2)) ->
    Int.equal min1 min2 && Int.equal max1 max2
  | (Fvariadic (min1, param1), Fvariadic (min2, param2)) ->
    Int.equal min1 min2 && Int.equal 0 (ft_params_compare [param1] [param2])
  | (Fellipsis (min1, pos1), Fellipsis (min2, pos2)) ->
    Int.equal min1 min2 && Pos.equal pos1 pos2
  | (Fstandard _, (Fvariadic _ | Fellipsis _))
  | (Fvariadic _, (Fstandard _ | Fellipsis _))
  | (Fellipsis _, (Fstandard _ | Fvariadic _)) ->
    false

let is_type_no_return = equal_locl_ty_ (Tprim Aast.Tnoreturn)

let make_function_type_rxvar param_ty =
  match param_ty with
  | (r, Tfun tfun) -> (r, Tfun { tfun with ft_reactive = RxVar None })
  | (r, Toption (r1, Tfun tfun)) ->
    (r, Toption (r1, Tfun { tfun with ft_reactive = RxVar None }))
  | _ -> param_ty

let rec equal_decl_ty_ ty_1 ty_2 =
  match (ty_1, ty_2) with
  | (Tany _, Tany _) -> true
  | (Terr, Terr) -> true
  | (Tthis, Tthis) -> true
  | (Tmixed, Tmixed) -> true
  | (Tnothing, Tnothing) -> true
  | (Tnonnull, Tnonnull) -> true
  | (Tdynamic, Tdynamic) -> true
  | (Tapply (id1, tyl1), Tapply (id2, tyl2)) ->
    Aast.equal_sid id1 id2 && equal_decl_tyl tyl1 tyl2
  | (Tgeneric s1, Tgeneric s2) -> String.equal s1 s2
  | (Taccess (ty1, idl1), Taccess (ty2, idl2)) ->
    equal_decl_ty ty1 ty2 && List.equal ~equal:Aast.equal_sid idl1 idl2
  | (Tarray (tk1, tv1), Tarray (tk2, tv2)) ->
    Option.equal equal_decl_ty tk1 tk2 && Option.equal equal_decl_ty tv1 tv2
  | (Tdarray (tk1, tv1), Tdarray (tk2, tv2)) ->
    equal_decl_ty tk1 tk2 && equal_decl_ty tv1 tv2
  | (Tvarray ty1, Tvarray ty2) -> equal_decl_ty ty1 ty2
  | (Tvarray_or_darray (tk1, tv1), Tvarray_or_darray (tk2, tv2)) ->
    Option.equal equal_decl_ty tk1 tk2 && equal_decl_ty tv1 tv2
  | (Tlike ty1, Tlike ty2) -> equal_decl_ty ty1 ty2
  | (Tprim ty1, Tprim ty2) -> Aast.equal_tprim ty1 ty2
  | (Toption ty, Toption ty2) -> equal_decl_ty ty ty2
  | (Tfun fty1, Tfun fty2) -> equal_decl_fun_type fty1 fty2
  | (Tunion tyl1, Tunion tyl2)
  | (Tintersection tyl1, Tintersection tyl2)
  | (Ttuple tyl1, Ttuple tyl2) ->
    equal_decl_tyl tyl1 tyl2
  | (Tshape (shape_kind1, fields1), Tshape (shape_kind2, fields2)) ->
    equal_shape_kind shape_kind1 shape_kind2
    && List.equal
         ~equal:(fun (k1, v1) (k2, v2) ->
           Ast_defs.ShapeField.equal k1 k2 && equal_shape_field_type v1 v2)
         (Nast.ShapeMap.elements fields1)
         (Nast.ShapeMap.elements fields2)
  | (Tpu_access (ty1, id1), Tpu_access (ty2, id2)) ->
    equal_decl_ty ty1 ty2 && Aast.equal_sid id1 id2
  | (Tvar v1, Tvar v2) -> Ident.equal v1 v2
  | (Tany _, _)
  | (Terr, _)
  | (Tthis, _)
  | (Tapply _, _)
  | (Tgeneric _, _)
  | (Taccess _, _)
  | (Tarray _, _)
  | (Tdarray _, _)
  | (Tvarray _, _)
  | (Tvarray_or_darray _, _)
  | (Tmixed, _)
  | (Tnothing, _)
  | (Tlike _, _)
  | (Tnonnull, _)
  | (Tdynamic, _)
  | (Toption _, _)
  | (Tprim _, _)
  | (Tfun _, _)
  | (Ttuple _, _)
  | (Tshape _, _)
  | (Tpu_access _, _)
  | (Tvar _, _)
  | (Tunion _, _)
  | (Tintersection _, _) ->
    false

and equal_decl_ty ty1 ty2 = equal_decl_ty_ (snd ty1) (snd ty2)

and equal_shape_field_type sft1 sft2 =
  equal_decl_ty sft1.sft_ty sft2.sft_ty
  && Bool.equal sft1.sft_optional sft2.sft_optional

and equal_decl_fun_arity a1 a2 =
  match (a1, a2) with
  | (Fstandard (min1, max1), Fstandard (min2, max2)) ->
    Int.equal min1 min2 && Int.equal max1 max2
  | (Fvariadic (min1, param1), Fvariadic (min2, param2)) ->
    Int.equal min1 min2 && equal_decl_ft_params [param1] [param2]
  | (Fellipsis (min1, pos1), Fellipsis (min2, pos2)) ->
    Int.equal min1 min2 && Pos.equal pos1 pos2
  | (Fstandard _, (Fvariadic _ | Fellipsis _))
  | (Fvariadic _, (Fstandard _ | Fellipsis _))
  | (Fellipsis _, (Fstandard _ | Fvariadic _)) ->
    false

and equal_decl_fun_type fty1 fty2 =
  equal_decl_possibly_enforced_ty fty1.ft_ret fty2.ft_ret
  && equal_decl_ft_params fty1.ft_params fty2.ft_params
  && Bool.equal fty1.ft_is_coroutine fty2.ft_is_coroutine
  && equal_decl_fun_arity fty1.ft_arity fty2.ft_arity
  && equal_reactivity fty1.ft_reactive fty2.ft_reactive
  && Bool.equal fty1.ft_return_disposable fty2.ft_return_disposable
  && Option.equal equal_param_mutability fty1.ft_mutability fty2.ft_mutability
  && Bool.equal fty1.ft_returns_mutable fty2.ft_returns_mutable

and equal_reactivity r1 r2 =
  match (r1, r2) with
  | (Nonreactive, Nonreactive) -> true
  | (Local ty1, Local ty2)
  | (Shallow ty1, Shallow ty2)
  | (Reactive ty1, Reactive ty2) ->
    Option.equal equal_decl_ty ty1 ty2
  | (MaybeReactive r1, MaybeReactive r2) -> equal_reactivity r1 r2
  | (RxVar r1, RxVar r2) -> Option.equal equal_reactivity r1 r2
  | (Nonreactive, _)
  | (Local _, _)
  | (Shallow _, _)
  | (Reactive _, _)
  | (MaybeReactive _, _)
  | (RxVar _, _) ->
    false

and equal_param_rx_annotation pa1 pa2 =
  match (pa1, pa2) with
  | (Param_rx_var, Param_rx_var) -> true
  | (Param_rx_if_impl ty1, Param_rx_if_impl ty2) -> equal_decl_ty ty1 ty2
  | (Param_rx_var, Param_rx_if_impl _)
  | (Param_rx_if_impl _, Param_rx_var) ->
    false

and equal_decl_tyl tyl1 tyl2 = List.equal ~equal:equal_decl_ty tyl1 tyl2

and equal_decl_possibly_enforced_ty ety1 ety2 =
  equal_decl_ty ety1.et_type ety2.et_type
  && Bool.equal ety1.et_enforced ety2.et_enforced

and equal_decl_fun_param param1 param2 =
  equal_decl_possibly_enforced_ty param1.fp_type param2.fp_type
  && equal_param_mode param1.fp_kind param2.fp_kind
  && Bool.equal param1.fp_accept_disposable param2.fp_accept_disposable
  && Option.equal
       equal_param_mutability
       param1.fp_mutability
       param2.fp_mutability

and equal_decl_ft_params params1 params2 =
  List.equal ~equal:equal_decl_fun_param params1 params2

let equal_typeconst_abstract_kind ak1 ak2 =
  match (ak1, ak2) with
  | (TCAbstract ty1, TCAbstract ty2) -> Option.equal equal_decl_ty ty1 ty2
  | (TCPartiallyAbstract, TCPartiallyAbstract) -> true
  | (TCConcrete, TCConcrete) -> true
  | (TCAbstract _, _)
  | (TCPartiallyAbstract, _)
  | (TCConcrete, _) ->
    false

let equal_enum_type et1 et2 =
  equal_decl_ty et1.te_base et2.te_base
  && Option.equal equal_decl_ty et1.te_constraint et2.te_constraint

let equal_decl_where_constraint c1 c2 =
  let (tya1, ck1, tyb1) = c1 in
  let (tya2, ck2, tyb2) = c2 in
  equal_decl_ty tya1 tya2
  && Ast_defs.equal_constraint_kind ck1 ck2
  && equal_decl_ty tyb1 tyb2

let equal_decl_tparam tp1 tp2 =
  Ast_defs.equal_variance tp1.tp_variance tp2.tp_variance
  && Ast_defs.equal_id tp1.tp_name tp2.tp_name
  && List.equal
       ~equal:
         (Tuple.T2.equal ~eq1:Ast_defs.equal_constraint_kind ~eq2:equal_decl_ty)
       tp1.tp_constraints
       tp2.tp_constraints
  && Aast.equal_reify_kind tp1.tp_reified tp2.tp_reified
  && List.equal
       ~equal:Nast.equal_user_attribute
       tp1.tp_user_attributes
       tp2.tp_user_attributes

let equal_typedef_type tt1 tt2 =
  Pos.equal tt1.td_pos tt2.td_pos
  && Aast.equal_typedef_visibility tt1.td_vis tt2.td_vis
  && List.equal ~equal:equal_decl_tparam tt1.td_tparams tt2.td_tparams
  && Option.equal equal_decl_ty tt1.td_constraint tt2.td_constraint
  && equal_decl_ty tt1.td_type tt2.td_type

let equal_fun_elt fe1 fe2 =
  Option.equal String.equal fe1.fe_deprecated fe2.fe_deprecated
  && equal_decl_ty fe1.fe_type fe2.fe_type
  && Pos.equal fe1.fe_pos fe2.fe_pos
