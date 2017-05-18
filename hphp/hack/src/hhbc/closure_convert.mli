(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* State used for closure conversion *)
type state

(* Initial state, given total number of ordinary classes in the program *)
val initial_state : int -> state

(* Get the closure classes at the end of closure conversion *)
val get_closure_classes : state -> Ast.class_ list

(* Convert functions, classes, or an entire program *)
val convert_fun : state -> Ast.fun_ -> state * Ast.fun_
val convert_class : state -> Ast.class_ -> state * Ast.class_
val convert_toplevel_prog : state -> Ast.program -> state * Ast.program
