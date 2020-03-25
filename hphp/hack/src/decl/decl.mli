(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * This function works by side effects. It is adding in the
 * Naming_table the nast produced from the ast passed as a parameter
 * (the SharedMem must thus have been initialized via SharedMem.init
 * prior to calling this function). It also assumes the Parser_heap
 * has been previously populated. It also adds dependencies
 * via Typing_deps.add_idep. It finally adds all the typing information
 * about classes, functions, typedefs, respectively in the globals
 * in Typing_env.Class, Typing_env.Fun, and Typing_env.Typedef.
 *)
val name_and_declare_types_program :
  sh:SharedMem.uses -> Provider_context.t -> Nast.program -> unit

val make_env :
  sh:SharedMem.uses -> Provider_context.t -> Relative_path.t -> unit

val fun_decl_in_env :
  Decl_env.env -> is_lambda:bool -> Nast.fun_ -> Typing_defs.fun_elt

val declare_const_in_file :
  write_shmem:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Typing_defs.decl_ty * Errors.t

val declare_record_def_in_file :
  write_shmem:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Typing_defs.record_def_type

val declare_typedef_in_file :
  write_shmem:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Typing_defs.typedef_type

val declare_class_in_file :
  sh:SharedMem.uses ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Decl_defs.decl_class_type option

val declare_fun_in_file :
  write_shmem:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Typing_defs.fun_elt

val start_tracking : unit -> unit

val stop_tracking : unit -> FileInfo.names
