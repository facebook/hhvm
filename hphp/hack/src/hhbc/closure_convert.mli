(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Environment used for closure conversion *)
type env

(* Initial environment, given total number of ordinary classes in the program *)
val initial_env : int -> env

(* Get the closure classes at the end of closure conversion *)
val get_closure_classes : env -> Ast.class_ list

(* Convert functions, classes, or an entire program *)
val convert_fun : env -> Ast.fun_ -> env * Ast.fun_
val convert_class : env -> Ast.class_ -> env * Ast.class_
val convert_block : env -> Ast.block -> env * Ast.block
val convert_prog : env -> Ast.program -> env * Ast.program
