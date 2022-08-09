(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* module HS = Hips_solver *)
module HT = Hips_types
module ST = Shape_analysis_types
module SS = Shape_analysis_solver

module Intra_shape :
  HT.Intra
    with type intra_entity = ST.entity_
     and type intra_constraint = ST.constraint_
     and type inter_constraint = ST.inter_constraint_
     and type any_constraint = ST.any_constraint = struct
  type intra_entity = ST.entity_

  type intra_constraint = ST.constraint_

  type inter_constraint = ST.inter_constraint_

  type any_constraint = ST.any_constraint

  let is_same_entity = SS.is_same_entity

  let max_iteration = 5

  let equiv = SS.equiv

  let substitute_inter_intra = SS.substitute_inter_intra

  let deduce = SS.deduce
end

(*
module Inter_shape = HS.Inter (Intra_shape)
let analyse = Inter_shape.analyse
*)
