(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs_core

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

type has_member_method = {
  hmm_explicit_targs: Nast.targ list; [@opaque]
      (** he list of explicit type arguments provided to the method call *)
  hmm_env_capability: locl_ty;
      (** A type respresenting the capabilities provided by the environment at the
          point of the call *)
}
[@@deriving show]

type has_member = {
  hm_name: Nast.sid;
  hm_type: locl_ty;
  hm_class_id: Nast.class_id_;
      (** This is required to check ambiguous object access, where sometimes
          HHVM would access the private member of a parent class instead of the
          one from the current class. *)
  hm_method: has_member_method option;
      (* - For a "has-property" constraint, this is `None`
       * - For a "has-method" constraint, this is `Some hmm` *)
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

val mk_constraint_type : Reason.t * constraint_type_ -> constraint_type

val deref_constraint_type : constraint_type -> Reason.t * constraint_type_

val get_reason_i : internal_type -> Reason.t

val constraint_ty_compare :
  ?normalize_lists:bool ->
  constraint_type ->
  constraint_type ->
  Ppx_deriving_runtime.int

val get_var_i : internal_type -> Tvid.t option

val is_tyvar_i : internal_type -> bool

val is_has_member : constraint_type -> bool

val is_locl_type : internal_type -> bool

val reason : internal_type -> locl_phase Reason.t_

val is_constraint_type : internal_type -> bool

module InternalType : sig
  val get_var : internal_type -> Tvid.t option

  val is_var_v : internal_type -> v:Tvid.t -> bool

  val is_not_var_v : internal_type -> v:Tvid.t -> bool
end
