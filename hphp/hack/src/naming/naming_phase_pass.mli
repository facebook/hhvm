(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type 'ctx t

val top_down : 'ctx Aast_defs.Pass.t -> 'ctx t

val bottom_up : 'ctx Aast_defs.Pass.t -> 'ctx t

val mk_visitor :
  'ctx t list ->
  ('ctx -> (unit, unit) program -> (unit, unit) program)
  * ('ctx -> (unit, unit) class_ -> (unit, unit) class_)
  * ('ctx -> (unit, unit) fun_def -> (unit, unit) fun_def)
  * ('ctx -> (unit, unit) module_def -> (unit, unit) module_def)
  * ('ctx -> (unit, unit) gconst -> (unit, unit) gconst)
  * ('ctx -> (unit, unit) typedef -> (unit, unit) typedef)
  * ('ctx -> (unit, unit) stmt -> (unit, unit) stmt)
