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
  (** Output type of analyse: applying substitution repeatedly either
      converges, or diverges *)
  type solution =
    | Divergent of I.any_constraint list SMap.t
    | Convergent of I.any_constraint list SMap.t

  (** The call "analyse base" repeatedly applies constraint substitution with
      respect to "base" postcomposed with constraint deduction, beginning with
      "base" itself. It either finds a fixpoint, in which case it outputs
      "Convergent fp", or terminates early, in which case it outputs
      "Divergent p". *)
  val analyse : I.any_constraint list SMap.t -> verbose:bool -> solution
end
