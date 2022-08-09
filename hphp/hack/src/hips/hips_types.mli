(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs

type param_entity = A.id_ * int [@@deriving eq, ord, show]

type entity = Param of param_entity [@@deriving eq, ord, show]

type ('a, 'b) any_constraint_ =
  | Intra of 'a
  | Inter of 'b

type 'a inter_constraint_ = Arg of param_entity * 'a

(** Domain-specific intra-procedural data that can be used to instantiate an
    inter-procedural constraint solver. Examples we have in mind include the
    shape-like-dict analysis and the detection of function upcasts to dynamic. *)
module type Intra = sig
  (** This entity type models "p" in inter-procedural constraints of the shape
      "Arg("f", 0, p)" and "Ret("f", p)" *)
  type intra_entity

  (** Intra-procedural constraint type, e.g. Has_static_key(p, 'a', int) *)
  type intra_constraint

  (** Inter-procedural constraints type, e.g. "Arg("f", 1, q)" for f(_, q, _).
      TODO(T127947010): Add inter-procedural return type, e.g. "Ret(f, p)", if
      the function f returns p. *)
  type inter_constraint = intra_entity inter_constraint_

  (** The union of inter- and intra-procedural constraint types. For example,
      "Intra Has_static_key(f0, 'a', int)" or "Inter Arg("f", 1, p)". *)
  type any_constraint = (intra_constraint, inter_constraint) any_constraint_

  (** Verifies whether an entity is the nth argument of a given function.
      For instance, calling with ("f", 0) and "p" should result in "true",
      if p is the first argument of f, and "false" otherwise. *)
  val is_same_entity : entity -> intra_entity -> bool

  (** The maximum number of iterations constraint substitution should be
      performed without reaching a fixpoint. *)
  val max_iteration : int

  (** The constraint substitution fixpoint is defined up to the following
      equivalence of lists of constraints. *)
  val equiv : any_constraint list -> any_constraint list -> bool

  (** Substitutes the intra-procedural constraint in the second argument
      with respect to the inter-procedural constraint in the first argument.
      For instance, calling it with the first argument "("f", 0, p)" and
      the second argument "Has_static_key(q, 'a', int)" should result in
      "Has_static_key(p, 'a', int)", if q is the first argument of f, and
      "Has_static_key(q, 'a', int)" otherwise. *)
  val substitute_inter_intra :
    inter_constraint -> intra_constraint -> intra_constraint

  (** Deduces a simplified list of intra-procedural constraints.  *)
  val deduce : intra_constraint list -> intra_constraint list
end
