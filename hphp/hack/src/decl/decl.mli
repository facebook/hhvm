(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val nast_to_decls :
  Direct_decl_parser.decls ->
  Provider_context.t ->
  Nast.program ->
  Direct_decl_parser.decls

(*
 * This function works by side effects. It is adding in the Naming_table the
 * nast produced from the filename passed as a parameter (the SharedMem must
 * thus have been initialized via SharedMem.init prior to calling this
 * function). Its performance benefits if the Parser_heap has been previously
 * populated. It also adds dependencies via Typing_deps.add_idep. It finally
 * adds all the typing information about classes, functions, typedefs,
 * respectively in the globals in Typing_env.Class, Typing_env.Fun, and
 * Typing_env.Typedef.
 *)
val make_env :
  sh:SharedMem.uses -> Provider_context.t -> Relative_path.t -> unit
