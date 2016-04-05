(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*
 * This function works by side effects. It is adding in the
 * Naming_heap the nast produced from the ast passed as a parameter
 * (the SharedMem must thus have been initialized via SharedMem.init
 * prior to calling this function). It also assumes the Parser_heap
 * has been previously populated. It also adds dependencies
 * via Typing_deps.add_idep. It finally adds all the typing information
 * about classes, functions, typedefs, respectively in the globals
 * in Typing_env.Class, Typing_env.Fun, and Typing_env.Typedef.
 *)
val name_and_declare_types_program:
  TypecheckerOptions.t -> Ast.program -> unit

val make_env: TypecheckerOptions.t -> Relative_path.t -> unit

val class_decl:
  TypecheckerOptions.t -> Nast.class_ -> unit

val fun_decl : Nast.fun_ -> unit

val fun_decl_in_env:
  Decl_env.env -> Nast.fun_ -> Typing_defs.decl Typing_defs.fun_type

val declare_const_in_file:
  TypecheckerOptions.t -> Relative_path.t -> string -> unit

val declare_typedef_in_file:
  TypecheckerOptions.t -> Relative_path.t -> string -> unit

val declare_class_in_file:
  TypecheckerOptions.t -> Relative_path.t -> string -> unit

val declare_fun_in_file:
  TypecheckerOptions.t -> Relative_path.t -> string -> unit

val failures_from_errors: Errors.t -> Relative_path.t -> Relative_path.Set.t

val errors_and_failures: unit -> Errors.t * Relative_path.t list

val reset_errors: unit -> unit
