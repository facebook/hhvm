(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

(**
 * Common types shared between coroutine modules.
 *)

(**
 * Data extracted during the state machine generation step.
 *)
type t = {
  properties: string list;
  parameters: Syntax.parameter_declaration list;
}
