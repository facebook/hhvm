(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)
module EditableSyntax = Full_fidelity_editable_syntax

val rewrite_suspends:
  EditableSyntax.t ->
  (* (next_label * next_temp) * new_node *)
  (int * int) * EditableSyntax.t
