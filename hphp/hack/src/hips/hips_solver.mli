(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hips_types

(** Generates an inter-procedural constraint solver from domain-specific
    intra-procedural data. *)
module Inter (I : Intra) : sig
  (** Inter-procedural constraint type *)
  type inter_constraint = I.inter_constraint

  (** Intra-procedural constraint type *)
  type intra_constraint = I.intra_constraint

  (** Union of inter- and intra-procedural constraint types *)
  type any_constraint = I.any_constraint

  (** Output type of analyse: applying substitution repeatedly either
      converges, or diverges *)
  type solution =
    | Divergent of any_constraint list SMap.t
    | Convergent of any_constraint list SMap.t

  (** Verifies whether two dictionaries with constraint lists as values are
      equivalent *)
  val equiv : any_constraint list SMap.t -> any_constraint list SMap.t -> bool

  (** Substitutes the inter-procedural constraints of the second argument with
      respect to constraints in the first argument *)
  val substitute :
    base_constraint_map:any_constraint list SMap.t ->
    any_constraint list SMap.t ->
    any_constraint list SMap.t

  (** The call "analyse base" repeatedly applies constraint substitution with
      respect to "base" postcomposed with constraint deduction, beginning with
      "base" itself. It either finds a fixpoint, in which case it outputs
      "Convergent fp", or terminates early, in which case it outputs
      "Divergent p". *)
  val analyse : any_constraint list SMap.t -> solution
end
