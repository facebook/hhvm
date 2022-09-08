(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module ST = Shape_analysis_types
module SS = Shape_analysis_solver
module HT = Hips_types

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

  let is_same_entity = ST.equal_entity_

  let embed_entity = SS.embed_entity

  let max_iteration = 15

  let compare_any_constraint = ST.compare_any_constraint

  let equiv = SS.equiv

  let substitute_inter_intra_forwards = SS.substitute_inter_intra_forwards

  let substitute_inter_intra_backwards = SS.substitute_inter_intra_backwards

  let deduce = SS.deduce

  let subsets = SS.subsets
end
