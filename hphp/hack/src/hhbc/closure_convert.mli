(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Convert entire program *)
val convert_toplevel_prog :
  Ast.program ->
  (bool * Ast.def) list * Emit_env.global_state
