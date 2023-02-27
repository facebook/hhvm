(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sdt_analysis_types

let decorated_constraint
    ~verbosity { origin; hack_pos; decorated_data = constraint_ } =
  let line = Pos.line hack_pos in
  let constraint_ = Constraint.show constraint_ in
  if verbosity > 0 then
    Format.asprintf "%4d: %4d: %s" line origin constraint_
  else
    Format.asprintf "%4d: %s" line constraint_
