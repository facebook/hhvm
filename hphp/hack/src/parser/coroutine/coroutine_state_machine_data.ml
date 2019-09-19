(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

(**
 * Common types shared between coroutine modules.
 *)

(**
 * Data extracted during the state machine generation step.
 *)

type parameter = {
  parameter_name: Syntax.t;
  parameter_declaration: Syntax.syntax;
}

type t = {
  properties: string list;
  parameters: parameter list;
}
