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
  | Vprotected_internal of {
      class_id: string;
      module_: string;
    }
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

(** The origin of a type is a succinct key that is unique to the
    type containing it. Consequently, two types with the same
    origin are necessarily identical. Any change to a type with
    origin needs to come with a *reset* of its origin. For example,
    all type mappers have to reset origins to [Missing_origin]. *)
type type_origin =
  | Missing_origin
      (** When we do not have any origin for the type. It is always
          correct to use [Missing_origin]; so when in doubt, use it. *)
  | From_alias of string * Pos_or_decl.t option
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
  | TSFregex_group of pos_string
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
  ua_raw_val: string option;
}
[@@deriving eq, show]

type 'ty tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: pos_id;
  tp_constraints: (Ast_defs.constraint_kind * 'ty) list;
  tp_reified: Aast.reify_kind;
  tp_user_attributes: user_attribute list;
}
[@@deriving eq, show, map]

type 'ty where_constraint = 'ty * Ast_defs.constraint_kind * 'ty
[@@deriving eq, show, map]

type enforcement =
  | Unenforced  (** The consumer doesn't enforce the type at runtime *)
  | Enforced  (** The consumer enforces the type at runtime *)
[@@deriving eq, show, ord]

(** Because Tfun is currently used as both a decl and locl ty, without this,
  the HH\Contexts\defaults alias must be stored in shared memory for a
  decl Tfun record. We can eliminate this if the majority of usages end up
  explicit or if we separate decl and locl Tfuns. *)
type 'ty capability =
  | CapDefaults of Pos_or_decl.t  (** Should not be used for lambda inference *)
  | CapTy of 'ty
[@@deriving hash, show, map]

(** Companion to fun_params type, intended to consolidate checking of
  implicit params for functions. *)
type 'ty fun_implicit_params = { capability: 'ty capability }
[@@deriving hash, show, map]

type 'ty fun_param = {
  fp_pos: Pos_or_decl.t;
  fp_name: string option;
  fp_type: 'ty;
  fp_flags: Typing_defs_flags.FunParam.t;
  fp_def_value: string option;
}
[@@deriving hash, show, map]

type 'ty fun_params = 'ty fun_param list [@@deriving hash, map]

(** The type of a function AND a method *)
type 'ty fun_type = {
  ft_tparams: 'ty tparam list;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_implicit_params: 'ty fun_implicit_params;
  ft_ret: 'ty;  (** Carries through the sync/async information from the aast *)
  ft_flags: Typing_defs_flags.Fun.t;
  ft_cross_package: cross_package_decl;
  ft_instantiated: bool;
}
[@@deriving hash, show, map]

(** = Reason.t * 'phase ty_ *)
type 'phase ty = ('phase Reason.t_[@transform.opaque]) * 'phase ty_

and type_tag_generic =
  | Filled of locl_phase ty
  | Wildcard of int

and type_tag =
  | BoolTag
  | IntTag
  | StringTag
  | ArraykeyTag
  | FloatTag
  | NumTag
  | ResourceTag
  | NullTag
  | ClassTag of Ast_defs.id_ * type_tag_generic list

and shape_field_predicate = {
  (* T196048813 *)
  (* sfp_optional: bool; *)
  sfp_predicate: type_predicate;
}

and shape_predicate = {
  (* T196048813 *)
  (* sp_allows_unknown_fields: bool; *)
  sp_fields: shape_field_predicate TShapeMap.t;
}

(* TODO optional and variadic components T201398626 T201398652 *)
and tuple_predicate = { tp_required: type_predicate list }

(** Represents the predicate of a type switch, i.e. in the expression
      ```
      if ($x is Bool) { ... } else { ... }
      ```

    The predicate would be `is Bool`
  *)
and type_predicate_ =
  | IsTag of type_tag
  | IsTupleOf of tuple_predicate
  | IsShapeOf of shape_predicate

and type_predicate = (Reason.t[@transform.opaque]) * type_predicate_

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
  | Tapply : (pos_id[@transform.opaque]) * decl_phase ty list -> decl_phase ty_
      (** Either an object type or a type alias, ty list are the arguments *)
  | Trefinement : decl_phase ty * decl_phase class_refinement -> decl_phase ty_
      (** 'With' refinements of the form `_ with { type T as int; type TC = C; }`. *)
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
  | Twildcard : decl_phase ty_
      (** Various intepretations, depending on context.
        *   inferred type e.g. (vec<_> $x) ==> $x[0]
        *   placeholder in refinement e.g. $x as Vector<_>
        *   placeholder for higher-kinded formal type parameter e.g. foo<T1<_>>(T1<int> $_)
        *)
  | Tlike : decl_phase ty -> decl_phase ty_
  (*========== Following Types Exist in Both Phases ==========*)
  | Tany : (TanySentinel.t[@transform.opaque]) -> 'phase ty_
  | Tnonnull : 'phase ty_
  | Tdynamic : 'phase ty_
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
  | Tprim : (Ast_defs.tprim[@transform.opaque]) -> 'phase ty_
      (** All the primitive types: int, string, void, etc. *)
  | Tfun : 'phase ty fun_type -> 'phase ty_
      (** A wrapper around fun_type, which contains the full type information for a
       * function, method, lambda, etc. *)
  | Ttuple : 'phase tuple_type -> 'phase ty_
      (** A wrapper around tuple_type, which contains information about tuple elements *)
  | Tshape : 'phase shape_type -> 'phase ty_
  | Tgeneric : string -> 'phase ty_
      (** The type of a generic parameter. The constraints on a generic parameter
       * are accessed through the lenv.tpenv component of the environment, which
       * is set up when checking the body of a function or method. See uses of
       * Typing_phase.add_generic_parameters_and_constraints.
       *)
  | Tunion : 'phase ty list -> 'phase ty_ [@transform.explicit]
      (** Union type.
       * The values that are members of this type are the union of the values
       * that are members of the components of the union.
       * Some examples (writing | for binary union)
       *   Tunion []  is the "nothing" type, with no values
       *   Tunion [int;float] is the same as num
       *   Tunion [null;t] is the same as Toption t
       *)
  | Tintersection : 'phase ty list -> 'phase ty_
  | Tvec_or_dict : 'phase ty * 'phase ty -> 'phase ty_
      (** Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>" *)
  | Taccess : 'phase taccess_type -> 'phase ty_
      (** Name of class, name of type const, remaining names of type consts *)
  | Tclass_ptr : 'phase ty -> 'phase ty_
      (** A type of a class pointer, class<T>. To be compatible with classname<T>,
        * it takes an arbitrary type. In the future, it should only take a string
        * that is a class name, and be named Tclass. The current Tclass would be
        * renamed to Tinstance, where a Tinstance is an instantiation of a Tclass *)
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)
  | Tvar : (Tvid.t[@transform.opaque]) -> locl_phase ty_
  | Tnewtype : string * locl_phase ty list * locl_phase ty -> locl_phase ty_
      (** The type of an opaque type or enum. Outside their defining files or
        when they represent enums, they are "opaque", which means that they
        only unify with themselves. Within a file, uses of newtypes are
        expanded to their definitions (unless the newtype is an enum).

        However, it is possible to have a constraint that allows us to relax
        opaqueness. For example:

          newtype MyType as int = ...

        or

          enum MyType: int as int { ... }

        Outside of the file where the type was defined, this translates to:

          Tnewtype ((pos, "MyType"), [], Tprim Tint)

        which means that MyType is abstract, but is a subtype of int as well.
        When the constraint is omitted, the third parameter is set to mixed.

        The second parameter is the list of type arguments to the type.
       *)
  | Tdependent :
      (dependent_type[@transform.opaque]) * locl_phase ty
      -> locl_phase ty_  (** see dependent_type *)
  | Tclass :
      (pos_id[@transform.opaque]) * exact * locl_phase ty list
      -> locl_phase ty_
      (** An instance of a class or interface, ty list are the arguments
       * If exact=Exact, then this represents instances of *exactly* this class
       * If exact=Nonexact, this also includes subclasses
       * TODO(T199606542) rename this to Tinstance *)
  | Tneg : (type_predicate[@transform.opaque]) -> locl_phase ty_
      (** The negation of the [type_predicate] *)
  | Tlabel : string -> locl_phase ty_
      (** The type of the label expression #ID *)

and 'phase taccess_type = 'phase ty * (pos_id[@transform.opaque])

and exact =
  | Exact
  | Nonexact of locl_phase class_refinement

(** Class refinements are for type annotations like

      Box with {type T = string}
  *)
and 'phase class_refinement = { cr_consts: 'phase refined_const SMap.t }

and 'phase refined_const = {
  rc_bound: 'phase refined_const_bound;
  rc_is_ctx: bool;
}

and 'phase refined_const_bound =
  | TRexact : 'phase ty -> 'phase refined_const_bound
      (** for `=` constraints *)
  | TRloose : 'phase refined_const_bounds -> 'phase refined_const_bound
      (** for `as` or `super` constraints *)

and 'phase refined_const_bounds = {
  tr_lower: 'phase ty list;
  tr_upper: 'phase ty list;
}

(** Whether all fields of this shape are known, types of each of the
 * known arms. *)
and 'phase shape_type = {
  s_origin: type_origin; [@transform.opaque]
  s_unknown_value: 'phase ty;
  s_fields: 'phase shape_field_type TShapeMap.t;
}

(**
  Required and extra components of a tuple. Extra components
  are either optional + variadic, or a type splat.
  Exmaple 1:
    (string,bool,optional float,optional bool,int...)
  has require components string, bool, optional components float, bool
  and variadic component int.
  Example 2:
    (string,float,...T)
  has required components string, float, and splat component T.
*)
and 'phase tuple_type = {
  t_required: 'phase ty list;
  t_extra: 'phase tuple_extra;
}

and 'phase tuple_extra =
  | Textra of {
      t_optional: 'phase ty list;
      t_variadic: 'phase ty;
    }
  | Tsplat of 'phase ty
[@@deriving hash, transform]

type decl_ty = decl_phase ty [@@deriving hash]

type locl_ty = locl_phase ty [@@deriving hash]

val equal_decl_ty : decl_ty -> decl_ty -> bool

val equal_type_predicate : type_predicate -> type_predicate -> bool

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

  val get_fp_splat : 'a fun_param -> bool

  val make_fp_flags :
    mode:param_mode ->
    accept_disposable:bool ->
    is_optional:bool ->
    readonly:bool ->
    ignore_readonly_error:bool ->
    splat:bool ->
    named:bool ->
    Typing_defs_flags.FunParam.t

  val get_fp_accept_disposable : 'a fun_param -> bool

  val get_fp_is_optional : 'a fun_param -> bool

  val get_fp_ignore_readonly_error : 'a fun_param -> bool

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

  val show_type_predicate_ : type_predicate_ -> string
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
[@@deriving show]

(* A can_index constraint represents the ability to do an array index operation.
 * We should have t <: { ci_key; ci_val; _ } when t is a type that supports
 * being index with a value of type ci_key, and will return a value of ci_val.
 *)
type can_index = {
  ci_key: locl_ty;
  ci_val: locl_ty;
  ci_index_expr: Nast.expr;
  ci_lhs_of_null_coalesce: bool;
  ci_expr_pos: Pos.t;
  ci_array_pos: Pos.t;
  ci_index_pos: Pos.t;
}
[@@deriving show]

(* `$source[key] = write` update source to type val *)
type can_index_assign = {
  cia_key: locl_ty;
  cia_write: locl_ty;
  cia_val: locl_ty;
  cia_index_expr: Nast.expr;
  cia_expr_pos: Pos.t;
  cia_array_pos: Pos.t;
  cia_index_pos: Pos.t;
  cia_write_pos: Pos.t;
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
  | Thas_const of {
      name: string;
      ty: locl_ty;
    }
      (** Check if the given type has a class constant that is compatible with [ty] *)
  | Tcan_index of can_index
  | Tcan_index_assign of can_index_assign
  | Tcan_traverse of can_traverse
  | Tdestructure of destructure
      (** The type of container destructuring via list() or splat `...`
          Implements valid destructuring operations via subtyping. *)
  | Ttype_switch of {
      predicate: type_predicate;
      ty_true: locl_ty;
      ty_false: locl_ty;
    }
      (** The type of a value we want to decompose based on a runtime type test.
          In the expression:
          ```
          if ($x is P) { ... } else { ... }
          ```

          The term `$x` must satisfy the constraint type_switch(P, T_true, T_false), where
          T_true is the type of `$x` if the predicate is true and T_false is the
          type of `$x` if the predicate is false
          *)
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

module Locl_subst : sig
  type t = locl_ty SMap.t

  val apply :
    locl_ty ->
    subst:locl_ty SMap.t ->
    combine_reasons:
      (src:Typing_reason.t -> dest:Typing_reason.t -> Typing_reason.t) ->
    locl_ty

  val apply_fun :
    locl_phase ty fun_type ->
    subst:locl_ty SMap.t ->
    combine_reasons:
      (src:Typing_reason.t -> dest:Typing_reason.t -> Typing_reason.t) ->
    locl_phase ty fun_type
end

(** Find the first element of a [locl_ty] satisfying the predicate [p] *)
val find_locl_ty : locl_ty -> p:(locl_ty -> bool) -> locl_ty option

(** Transform a decl ty top down *)
val transform_top_down_decl_ty :
  decl_ty ->
  on_ty:
    (decl_ty ->
    ctx:'a ->
    'a * [< `Continue of decl_ty | `Stop of decl_ty | `Restart of decl_ty ]) ->
  on_rc_bound:
    (decl_phase refined_const_bound ->
    ctx:'a ->
    'a
    * [< `Continue of decl_phase refined_const_bound
      | `Stop of decl_phase refined_const_bound
      | `Restart of decl_phase refined_const_bound
      ]) ->
  ctx:'a ->
  decl_ty

val is_type_tag_generic_wildcard : type_tag_generic -> bool
