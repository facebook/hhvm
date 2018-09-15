(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
  Exception representing not finding a class during decl
*)
exception Decl_not_found of string

val fun_reactivity :
  Decl_env.env ->
  Nast.user_attribute list ->
  Typing_defs.reactivity

val condition_type_from_attributes:
  Decl_env.env ->
  Nast.user_attribute list ->
  Typing_defs.decl Typing_defs.ty option

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
