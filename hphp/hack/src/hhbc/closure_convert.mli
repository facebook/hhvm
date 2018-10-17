(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

type hoist_kind = TopLevel | Hoisted

type convert_result = {
  ast_defs: (hoist_kind * Ast.def) list;
  global_state: Emit_env.global_state;
  strict_types: bool option;
}

(* Convert entire program *)
val convert_toplevel_prog :
  popt:ParserOptions.t ->
  Ast.program ->
  convert_result
