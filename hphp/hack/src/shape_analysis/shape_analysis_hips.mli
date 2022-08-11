(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module HT = Hips_types
module ST = Shape_analysis_types

module Intra_shape :
  HT.Intra
    with type intra_entity = ST.entity_
     and type intra_constraint = ST.constraint_
     and type inter_constraint = ST.inter_constraint_
     and type any_constraint = ST.any_constraint
