(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Reason = Typing_reason
module SN = Naming_special_names

type visibility =
  | Vpublic
  | Vprivate of string
  | Vprotected of string [@@deriving show]

type exact =
  | Exact
  | Nonexact

(* All the possible types, reason is a trace of why a type
   was inferred in a certain way.

   Types exists in two phases. Phase one is 'decl', meaning it is a type that
   was declared in user code. Phase two is 'locl', meaning it is a type that is
   inferred via local inference.
*)
(* create private types to represent the different type phases *)
type decl = private DeclPhase
type locl = private LoclPhase

let show_phase_ty _ = "<phase_ty>"
let pp_phase_ty _ _ = Printf.printf "%s\n" "<phase_ty>"

type 'phase ty = ( Reason.t * 'phase ty_ )

(* A shape may specify whether or not fields are required. For example, consider
   this typedef:

     type ShapeWithOptionalField = shape(?'a' => ?int);

   With this definition, the field 'a' may be unprovided in a shape. In this
   case, the field 'a' would have sf_optional set to true.
   *)
and 'phase shape_field_type = {
  sft_optional : bool;
  sft_ty : 'phase ty;
}

and _ ty_ =
  (*========== Following Types Exist Only in the Declared Phase ==========*)
  (* The late static bound type of a class *)
  | Tthis : decl ty_

  (* Either an object type or a type alias, ty list are the arguments *)
  | Tapply : Nast.sid * decl ty list -> decl ty_

  (* The type of a generic parameter. The constraints on a generic parameter
   * are accessed through the lenv.tpenv component of the environment, which
   * is set up when checking the body of a function or method. See uses of
   * Typing_phase.localize_generic_parameters_with_bounds.
   *)
  | Tgeneric : string -> decl ty_

  (* Name of class, name of type const, remaining names of type consts *)
  | Taccess : taccess_type -> decl ty_

  (* The type of the various forms of "array":
   * Tarray (None, None)         => "array"
   * Tarray (Some ty, None)      => "array<ty>"
   * Tarray (Some ty1, Some ty2) => "array<ty1, ty2>"
   * Tarray (None, Some ty)      => [invalid]
   *)
  | Tarray : decl ty option * decl ty option -> decl ty_

  (* Tdarray (ty1, ty2) => "darray<ty1, ty2>" *)
  | Tdarray : decl ty * decl ty -> decl ty_

  (* Tvarray (ty) => "varray<ty>" *)
  | Tvarray : decl ty -> decl ty_

  (* Tvarray_or_darray (ty) => "varray_or_darray<ty>" *)
  | Tvarray_or_darray : decl ty -> decl ty_

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
   * mixed exists only in the decl phase because it is desugared into ?nonnull
   * during the localization phase.
   *)
  | Tmixed : decl ty_

  | Tnothing : decl ty_

  | Tlike : decl ty -> decl ty_

  (*========== Following Types Exist in Both Phases ==========*)
  | Tany
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
  | Terr

  (* Nullable, called "option" in the ML parlance. *)
  | Toption : 'phase ty -> 'phase ty_

  (* All the primitive types: int, string, void, etc. *)
  | Tprim : Nast.tprim -> 'phase ty_

  (* A wrapper around fun_type, which contains the full type information for a
   * function, method, lambda, etc. Note that lambdas have an additional layer
   * of indirection before you get to Tfun -- see Tanon below. *)
  | Tfun : 'phase fun_type -> 'phase ty_


  (* Tuple, with ordered list of the types of the elements of the tuple. *)
  | Ttuple : 'phase ty list -> 'phase ty_

  (* Whether all fields of this shape are known, types of each of the
   * known arms.
   *)
  | Tshape
    : shape_fields_known * ('phase shape_field_type Nast.ShapeMap.t)
      -> 'phase ty_

  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)

  (* A type variable (not to be confused with a type parameter).
   * It represents a type that is "unknown" and must be inferred by Hack's
   * constraint-based type inference engine.
   *)
  | Tvar : Ident.t -> locl ty_

  (* The type of an opaque type (e.g. a "newtype" outside of the file where it
   * was defined). They are "opaque", which means that they only unify with
   * themselves. However, it is possible to have a constraint that allows us to
   * relax this. For example:
   *
   *   newtype my_type as int = ...
   *
   * Outside of the file where the type was defined, this translates to:
   *
   *   Tabstract (AKnewtype (pos, "my_type", []), Some (Tprim Tint))
   *
   * Which means that my_type is abstract, but is subtype of int as well.
   *
   * We also create abstract types for generic parameters of a function, i.e.
   *
   *   function foo<T>(T $x): void {
   *     // Body
   *   }
   *
   * The type 'T' will be represented as an abstract type when type checking
   * the body of 'foo'.
   *
   * Finally abstract types are also derived from the 'this' type and
   * accessing type constants on it, resulting in a dependent type.
   *)
  | Tabstract : abstract_kind * locl ty option -> locl ty_

  (* An anonymous function, including the fun arity, and the identifier to
   * type the body of the function. (The actual closure is stored in
   * Typing_env.env.genv.anons) *)
  | Tanon : locl fun_arity * Ident.t -> locl ty_

  (* Union type.
   * The values that are members of this type are the union of the values
   * that are members of the components of the union.
   * Some examples (writing | for binary union)
   *   Tunion []  is the "nothing" type, with no values
   *   Tunion [int;float] is the same as num
   *   Tunion [null;t] is the same as Toption t
   *)
  | Tunion : locl ty list -> locl ty_

  | Tintersection : locl ty list -> locl ty_

  (* Tobject is an object type compatible with all objects. This type is also
   * compatible with some string operations (since a class might implement
   * __toString), but not with string type hints. In a similar way, Tobject
   * is compatible with some array operations (since a class might implement
   * ArrayAccess), but not with array type hints.
   *
   * Tobject is currently used to type code like:
   *   ../test/typecheck/return_unknown_class.php
   *)
  | Tobject : locl ty_

  (* An instance of a class or interface, ty list are the arguments
   * If exact=Exact, then this represents instances of *exactly* this class
   * If exact=Nonexact, this also includes subclasses
   *)
  | Tclass : Nast.sid * exact * locl ty list -> locl ty_

  (* Localized version of Tarray *)
  | Tarraykind : array_kind -> locl ty_

and array_kind =
  (* Those three types directly correspond to their decl level counterparts:
   * array, array<_> and array<_, _> *)
  | AKany
  (* An array declared as a varray. *)
  | AKvarray of locl ty
  (* An array declared as a darray. *)
  | AKdarray of locl ty * locl ty
  (* An array annotated as a varray_or_darray. *)
  | AKvarray_or_darray of locl ty
  (* An array "used like a vec".
   * /!\ An actual `vec` is represented as a `Tclass ("\\vec", [...])`
   *)
  | AKvec of locl ty
  (* An array "used like a map or dict".
   * /!\ An actual `dict` is represented as a `Tclass ("\\dict", [...])`
   *)
  | AKmap of locl ty * locl ty
  (* This is a type created when we see array() literal *)
  | AKempty

(* An abstract type derived from either a newtype, a type parameter, or some
 * dependent type
 *)
and abstract_kind =
    (* newtype foo<T1, T2> ... *)
  | AKnewtype of string * locl ty list
    (* enum foo ... *)
  | AKenum of string
    (* <T super C> ; None if 'as' constrained *)
  | AKgeneric of string
    (* see dependent_type *)
  | AKdependent of dependent_type

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
and dependent_type =
  (* Type that is the subtype of the late bound type within a class. *)
  [ `this
  (* A class name, new type, or generic, i.e.
   *
   * abstract class C { abstract const type T }
   *
   * The type C::T is (`cls '\C', ['T'])
   *)
  | `cls of string
  (* A reference to some expression. For example:
   *
   *  $x->foo()
   *
   *  The expression $x would have a reference Ident.t
   *  The expression $x->foo() would have a different one
   *)
  | `expr of Ident.t
  ]

and taccess_type = decl ty * Nast.sid list

(* Local shape constructed using "shape" keyword has all the fields
 * known:
 *
 *   $s = shape('x' => 4, 'y' => 4);
 *
 * It has fields 'x' and 'y' and definitely no other fields. On the other
 * hand, shape types that come from typehints may (due to structural
 * subtyping of shapes) have some other, unlisted fields:
 *
 *   type s = shape('x' => int);
 *
 *   function f(s $s) {
 *   }
 *
 *   f(shape('x' => 4, 'y' => 5));
 *
 * The call to f is valid because of structural subtyping - shapes are
 * permitted to "forget" fields. But the 'y' field still exists at runtime,
 * and we cannot say inside the body of $f that we know that 'x' is the only
 * field. This is relevant when deciding if it's safe to omit optional fields
 * - if shape fields are not fully known, even optional fields have to be
 * explicitly set/unset.
 *
 * We also track in additional map of FieldsPartiallyKnown names of fields
 * that are known to not exist (because they were explicitly unset).
 *)
and shape_fields_known =
  | FieldsFullyKnown
  | FieldsPartiallyKnown of Pos.t Nast.ShapeMap.t

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
  | Local of decl ty option
  | Shallow of decl ty option
  | Reactive of decl ty option
  | MaybeReactive of reactivity
  | RxVar of reactivity option

and param_mutability =
  | Param_owned_mutable
  | Param_borrowed_mutable
  | Param_maybe_mutable

and fun_tparams_kind =
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

(* The type of a function AND a method.
 * A function has a min and max arity because of optional arguments *)
and 'phase fun_type = {
  ft_pos        : Pos.t               ;
  ft_deprecated : string option       ;
  ft_abstract   : bool                ;
  ft_is_coroutine : bool              ;
  ft_arity      : 'phase fun_arity    ;
  ft_tparams    : ('phase tparam) list * fun_tparams_kind;
  ft_where_constraints : 'phase where_constraint list  ;
  ft_params     : 'phase fun_params   ;
  ft_ret        : 'phase ty           ;
  (* Carries through the sync/async information from the aast *)
  ft_fun_kind   : Ast_defs.fun_kind        ;
  ft_reactive   : reactivity          ;
  ft_return_disposable : bool         ;
  (* mutability of the receiver *)
  ft_mutability : param_mutability option   ;
  ft_returns_mutable : bool           ;
  ft_decl_errors : Errors.t option    ;
  ft_returns_void_to_rx: bool         ;
}

(* Arity information for a fun_type; indicating the minimum number of
 * args expected by the function and the maximum number of args for
 * standard, non-variadic functions or the type of variadic argument taken *)
and 'phase fun_arity =
  | Fstandard of int * int (* min ; max *)
  (* PHP5.6-style ...$args finishes the func declaration *)
  | Fvariadic of int * 'phase fun_param (* min ; variadic param type *)
  (* HH-style ... anonymous variadic arg; body presumably uses func_get_args *)
  | Fellipsis of int * Pos.t  (* min ; position of ... *)

and param_mode =
  | FPnormal
  | FPref
  | FPinout

and param_rx_annotation =
  | Param_rx_var
  | Param_rx_if_impl of decl ty

and 'phase fun_param = {
  fp_pos  : Pos.t;
  fp_name : string option;
  fp_type : 'phase ty;
  fp_kind : param_mode;
  fp_accept_disposable : bool;
  fp_mutability           : param_mutability option;
  fp_rx_annotation: param_rx_annotation option;
}

and 'phase fun_params = 'phase fun_param list

and xhp_attr_tag =
  | Required
  | Lateinit

and xhp_attr = {
  xa_tag         : xhp_attr_tag option;
  xa_has_default : bool;
}

and class_elt = {
  ce_abstract    : bool;
  ce_final       : bool;
  ce_xhp_attr    : xhp_attr option;
  (* This field has different meanings in shallow mode and eager mode:
   * In shallow mode, true if this method has attribute __Override.
   * In eager mode, true if this method is originally defined in a trait,
   * AND has the override attribute, AND the trait does not inherit any
   * other method of that name. *)
  ce_override    : bool;
  (* true if this static property has attribute __LSB *)
  ce_lsb         : bool;
  (* true if this method has attribute __MemoizeLSB *)
  ce_memoizelsb  : bool;
  (* true if this elt arose from require-extends or other mechanisms
     of hack "synthesizing" methods that were not written by the
     programmer. The eventual purpose of this is to make sure that
     elts that *are* written by the programmer take precedence over
     synthesized elts. *)
  ce_synthesized : bool;
  ce_visibility  : visibility;
  ce_const       : bool;
  ce_lateinit    : bool;
  ce_type        : decl ty Lazy.t;
  (* identifies the class from which this elt originates *)
  ce_origin      : string;
}

and class_const = {
  cc_synthesized : bool;
  cc_abstract    : bool;
  cc_pos         : Pos.t;
  cc_type        : decl ty;
  cc_expr        : Nast.expr option;
  (* identifies the class from which this const originates *)
  cc_origin      : string;
}

(* The position is that of the hint in the `use` / `implements` AST node
 * that causes a class to have this requirement applied to it. E.g.
 *
 * class Foo {}
 *
 * interface Bar {
 *   require extends Foo; <- position of the decl ty
 * }
 *
 * class Baz extends Foo implements Bar { <- position of the `implements`
 * }
 *)
and requirement = Pos.t * decl ty

(* In the default case, we use Inconsistent. If a class has <<__ConsistentConstruct>>,
 * or if it inherits a class that has <<__ConsistentConstruct>>, we use inherited.
 * If we have a new final class that doesn't extend from <<__ConsistentConstruct>>,
 * then we use Final. Only classes that are Inconsistent or Final can have reified
 * generics. *)
and consistent_kind =
  | Inconsistent
  | ConsistentConstruct
  | FinalClass

and class_type = {
  tc_need_init           : bool;
  (* Whether the typechecker knows of all (non-interface) ancestors
   * and thus known all accessible members of this class *)
  tc_members_fully_known : bool;
  tc_abstract            : bool;
  tc_final               : bool;
  tc_const               : bool;
  (* True when the class is annotated with the __PPL attribute. *)
  tc_ppl                 : bool;
  (* When a class is abstract (or in a trait) the initialization of
   * a protected member can be delayed *)
  tc_deferred_init_members : SSet.t;
  tc_kind                : Ast_defs.class_kind;
  tc_is_xhp              : bool;
  tc_is_disposable       : bool;
  tc_name                : string ;
  tc_pos                 : Pos.t ;
  tc_tparams             : decl tparam list ;
  tc_consts              : class_const SMap.t;
  tc_typeconsts          : typeconst_type SMap.t;
  tc_props               : class_elt SMap.t;
  tc_sprops              : class_elt SMap.t;
  tc_methods             : class_elt SMap.t;
  tc_smethods            : class_elt SMap.t;
  (* the bool represents final constructor or __ConsistentConstruct *)
  tc_construct           : class_elt option * consistent_kind;
  (* This includes all the classes, interfaces and traits this class is
   * using. *)
  tc_ancestors           : decl ty SMap.t ;
  tc_req_ancestors       : requirement list;
  tc_req_ancestors_extends : SSet.t; (* the extends of req_ancestors *)
  tc_extends             : SSet.t;
  tc_enum_type           : enum_type option;
  tc_sealed_whitelist    : SSet.t option;
  tc_decl_errors         : Errors.t option;
}

and typeconst_abstract_kind =
  | TCAbstract of decl ty option
  | TCPartiallyAbstract
  | TCConcrete

and typeconst_type = {
  ttc_abstract    : typeconst_abstract_kind;
  ttc_name        : Nast.sid;
  ttc_constraint  : decl ty option;
  ttc_type        : decl ty option;
  ttc_origin      : string;
  ttc_enforceable : Pos.t * bool;
}

and enum_type = {
  te_base       : decl ty;
  te_constraint : decl ty option;
}

and typedef_type = {
  td_pos: Pos.t;
  td_vis: Nast.typedef_visibility;
  td_tparams: decl tparam list;
  td_constraint: decl ty option;
  td_type: decl ty;
  td_decl_errors: Errors.t option;
}

and 'phase tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: Ast_defs.id;
  tp_constraints: (Ast_defs.constraint_kind * 'phase ty) list;
  tp_reified: Nast.reify_kind;
  tp_user_attributes: Nast.user_attribute list;
}

and 'phase where_constraint =
  'phase ty * Ast_defs.constraint_kind * 'phase ty

type phase_ty =
  | DeclTy of decl ty
  | LoclTy of locl ty

type deserialization_error =
  (* The type was valid, but some component thereof was a decl ty when we
  expected a locl ty, or vice versa. *)
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
  type_expansions : (Pos.t * string) list;
  substs : locl ty SMap.t;
  this_ty : locl ty;
  (* The class that the type is extracted from. Used for creating expression
   * dependent types for type constants.
   *)
  from_class : Nast.class_id_ option;
  validate_dty : (expand_env -> decl ty -> unit) option;
}

let has_expanded {type_expansions; _} x =
  List.exists type_expansions begin function
    | (_, x') when x = x' -> true
    | _ -> false
  end

(* The identifier for this *)
let this = Local_id.make_scoped "$this"

let arity_min ft_arity : int = match ft_arity with
  | Fstandard (min, _) | Fvariadic (min, _) | Fellipsis (min, _) -> min

let get_param_mode ~is_ref callconv =
  (* If a param has both & and inout, this should have errored in parsing. *)
  match callconv with
  | Some Ast_defs.Pinout -> FPinout
  | None when is_ref -> FPref
  | None -> FPnormal

module AbstractKind = struct
  let to_string = function
    | AKnewtype (name, _) -> name
    | AKgeneric name -> name
    | AKenum name -> Utils.strip_ns name
    | AKdependent dt ->
       let dt =
         match dt with
         | `this -> SN.Typehints.this
         | `cls c -> c
         | `expr i ->
             let display_id = Reason.get_expr_display_id i in
             "<expr#"^string_of_int display_id^">" in
       dt

  let is_generic_dep_ty s = String_utils.is_substring "::" s
end

module ShapeFieldMap = struct
  include Nast.ShapeMap

  let map_and_rekey shape_map key_f value_f =
    let f_over_shape_field_type ({ sft_ty; _ } as shape_field_type) =
      { shape_field_type with sft_ty = value_f sft_ty } in
    Nast.ShapeMap.map_and_rekey
      shape_map
      key_f
      f_over_shape_field_type

  let map_env f env shape_map =
    let f_over_shape_field_type env _key ({ sft_ty; _ } as shape_field_type) =
      let env, sft_ty = f env sft_ty in
      env, { shape_field_type with sft_ty } in
    Nast.ShapeMap.map_env f_over_shape_field_type env shape_map

  let map f shape_map = map_and_rekey shape_map (fun x -> x) f

  let iter f shape_map =
    let f_over_shape_field_type shape_map_key { sft_ty; _ } =
      f shape_map_key sft_ty in
    Nast.ShapeMap.iter f_over_shape_field_type shape_map

  let iter_values f = iter (fun _ -> f)
end

module ShapeFieldList = struct
  include Hh_core.List

  let map_env env xs ~f =
    let f_over_shape_field_type env ({ sft_ty; _ } as shape_field_type) =
      let env, sft_ty = f env sft_ty in
      env, { shape_field_type with sft_ty } in
    Hh_core.List.map_env env xs ~f:f_over_shape_field_type
end

(*****************************************************************************)
(* Suggest mode *)
(*****************************************************************************)

(* Set to true when we are trying to infer the missing type hints. *)
let is_suggest_mode = ref false

(* Ordinal value for type constructor, for localized types *)
let ty_con_ordinal ty =
  match snd ty with
  | Tany | Terr -> 0
  | Toption (_, Tnonnull) -> 1
  | Tnonnull -> 2
  | Tdynamic -> 3
  | Toption _ -> 4
  | Tprim _ -> 5
  | Tfun _ -> 6
  | Ttuple _ -> 7
  | Tshape _ -> 8
  | Tvar _ -> 9
  | Tabstract _ -> 10
  | Tanon _ -> 11
  | Tunion _ -> 12
  | Tintersection _ -> 13
  | Tobject -> 14
  | Tclass _ -> 15
  | Tarraykind _ -> 16

let array_kind_con_ordinal ak =
  match ak with
  | AKany -> 0
  | AKvarray _ -> 1
  | AKvec _ -> 2
  | AKdarray _ -> 3
  | AKvarray_or_darray _ -> 4
  | AKmap _ -> 5
  | AKempty -> 6

let abstract_kind_con_ordinal ak =
  match ak with
  | AKnewtype _ -> 0
  | AKenum _ -> 1
  | AKgeneric _ -> 2
  | AKdependent _ -> 3

(* Compare two types syntactically, ignoring reason information and other
 * small differences that do not affect type inference behaviour. This
 * comparison function can be used to construct tree-based sets of types,
 * or to compare two types for "exact" equality.
 * Note that this function does *not* expand type variables, or type
 * aliases.
 * But if ty_compare ty1 ty2 = 0, then the types must not be distinguishable
 * by any typing rules.
 *)
let rec ty_compare ?(normalize_lists = false) ty1 ty2 =
  let rec ty_compare ty1 ty2 =
    let ty_1, ty_2 = (snd ty1, snd ty2) in
    match  ty_1, ty_2 with
      | Tprim ty1, Tprim ty2 ->
        compare ty1 ty2
      | Toption ty, Toption ty2 ->
        ty_compare ty ty2
      | Tfun fty, Tfun fty2 ->
        tfun_compare fty fty2
      | Tunion tyl1, Tunion tyl2
      | Tintersection tyl1, Tintersection tyl2 ->
        tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
      | Ttuple tyl1, Ttuple tyl2 ->
        tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
      | Tabstract (ak1, opt_cstr1), Tabstract (ak2, opt_cstr2) ->
        begin match abstract_kind_compare ak1 ak2 with
        | 0 -> opt_ty_compare opt_cstr1 opt_cstr2
        | n -> n
        end
      (* An instance of a class or interface, ty list are the arguments *)
      | Tclass (id, exact, tyl), Tclass(id2, exact2, tyl2) ->
        begin match String.compare (snd id) (snd id2) with
        | 0 ->
          begin match tyl_compare ~sort:false tyl tyl2 with
          | 0 ->
            begin match exact, exact2 with
            | Exact, Exact -> 0
            | Nonexact, Nonexact -> 0
            | Nonexact, Exact -> -1
            | Exact, Nonexact -> 1
            end
          | n -> n
          end
        | n -> n
        end
      | Tarraykind ak1, Tarraykind ak2 ->
        array_kind_compare ak1 ak2
      | Tshape (known1, fields1), Tshape (known2, fields2) ->
        begin match shape_fields_known_compare known1 known2 with
        | 0 ->
          List.compare (fun (k1,v1) (k2,v2) ->
            match Ast_defs.ShapeField.compare k1 k2 with
            | 0 -> shape_field_type_compare v1 v2
            | n -> n)
            (Nast.ShapeMap.elements fields1) (Nast.ShapeMap.elements fields2)
        | n -> n
        end
      | Tvar v1, Tvar v2 ->
        compare v1 v2
      | Tanon (_, id1), Tanon (_, id2) ->
        compare id1 id2
      | _ ->
        ty_con_ordinal ty1 - ty_con_ordinal ty2

    and shape_fields_known_compare sfk1 sfk2 =
      match sfk1, sfk2 with
      | FieldsFullyKnown, FieldsFullyKnown -> 0
      | FieldsFullyKnown, FieldsPartiallyKnown _ -> -1
      | FieldsPartiallyKnown _, FieldsFullyKnown -> 1
      | FieldsPartiallyKnown f1, FieldsPartiallyKnown f2 ->
        List.compare Ast_defs.ShapeField.compare
          (Nast.ShapeMap.keys f1) (Nast.ShapeMap.keys f2)

    and shape_field_type_compare sft1 sft2 =
      match ty_compare sft1.sft_ty sft2.sft_ty with
      | 0 -> compare sft1.sft_optional sft2.sft_optional
      | n -> n

    and tfun_compare fty1 fty2 =
      begin match ty_compare fty1.ft_ret fty2.ft_ret with
      | 0 ->
        begin match ft_params_compare fty1.ft_params fty2.ft_params with
        | 0 ->
          compare
            (fty1.ft_is_coroutine, fty1.ft_arity, fty1.ft_reactive,
            fty1.ft_return_disposable, fty1.ft_mutability, fty1.ft_returns_mutable)
            (fty2.ft_is_coroutine, fty2.ft_arity, fty2.ft_reactive,
            fty2.ft_return_disposable, fty2.ft_mutability, fty2.ft_returns_mutable)
        | n -> n
        end
      | n -> n
      end

    and opt_ty_compare opt_ty1 opt_ty2 =
      match opt_ty1, opt_ty2 with
      | None, None -> 0
      | Some _, None -> 1
      | None, Some _ -> -1
      | Some ty1, Some ty2 -> ty_compare ty1 ty2

    and array_kind_compare ak1 ak2 =
      match ak1, ak2 with
      | AKmap (ty1, ty2), AKmap (ty3, ty4)
      | AKdarray (ty1, ty2), AKdarray (ty3, ty4) ->
        tyl_compare ~sort:false [ty1; ty2] [ty3; ty4]
      | AKvarray ty1, AKvarray ty2
      | AKvarray_or_darray ty1, AKvarray_or_darray ty2
      | AKvec ty1, AKvec ty2 ->
        ty_compare ty1 ty2
      | _ ->
        array_kind_con_ordinal ak1 - array_kind_con_ordinal ak2

    in
    ty_compare ty1 ty2

and tyl_compare ~sort ?(normalize_lists = false) tyl1 tyl2 =
  let (tyl1, tyl2) = if sort
    then (List.sort ty_compare tyl1, List.sort ty_compare tyl2)
    else (tyl1, tyl2) in
  List.compare (ty_compare ~normalize_lists) tyl1 tyl2

and ft_params_compare ?(normalize_lists = false) params1 params2 =
  let ty_compare = ty_compare ~normalize_lists in

  let rec ft_params_compare params1 params2 =
    List.compare ft_param_compare params1 params2

  and ft_param_compare param1 param2 =
    match ty_compare param1.fp_type param2.fp_type with
    | 0 ->
      compare
        (param1.fp_kind, param1.fp_accept_disposable, param1.fp_mutability)
        (param2.fp_kind, param2.fp_accept_disposable, param2.fp_mutability)
    | n -> n

  in
  ft_params_compare params1 params2

and abstract_kind_compare ?(normalize_lists = false) t1 t2 =
  let tyl_compare = tyl_compare ~normalize_lists in
  match t1, t2 with
  | AKnewtype (id, tyl), AKnewtype (id2, tyl2) ->
    begin match String.compare id id2 with
    | 0 -> tyl_compare ~sort:false tyl tyl2
    | n -> n
    end
  | AKgeneric id1, AKgeneric id2
  | AKenum id1, AKenum id2 ->
    String.compare id1 id2
  | AKdependent d1, AKdependent d2 ->
    compare d1 d2
  | _ ->
    abstract_kind_con_ordinal t1 - abstract_kind_con_ordinal t2

let ty_equal ?(normalize_lists = false) ty1 ty2 =
  ty_compare ~normalize_lists ty1 ty2 = 0

let make_function_type_rxvar param_ty =
  match param_ty with
  | (r, Tfun tfun) ->
    r, Tfun { tfun with ft_reactive = RxVar None }
  | (r, Toption (r1, Tfun tfun)) ->
    r, Toption (r1, Tfun { tfun with ft_reactive = RxVar None })
  | _ ->
    param_ty
