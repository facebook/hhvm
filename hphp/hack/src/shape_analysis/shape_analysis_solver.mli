(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types
module HT = Hips_types

val deduce : constraint_ list -> constraint_ list

val produce_results :
  Typing_env_types.env -> constraint_ list -> shape_result list

val embed_entity : HT.entity -> entity_

(** Backwards substitutes the intra-procedural constraint in the second argument
    with respect to the inter-procedural constraint in the first argument *)
val substitute_inter_intra_backwards :
  inter_constraint_ -> constraint_ -> constraint_ option

(** Forwards substitutes the intra-procedural constraint in the second argument
    with respect to the inter-procedural constraint in the first argument *)
val substitute_inter_intra_forwards :
  inter_constraint_ -> constraint_ -> constraint_ option

val equiv : any_constraint list -> any_constraint list -> bool

val subsets : entity_ -> entity_ -> constraint_
