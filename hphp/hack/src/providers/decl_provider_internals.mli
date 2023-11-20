(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val find_in_direct_decl_parse :
  cache_results:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  (Shallow_decl_defs.decl -> 'a option) ->
  'a option

val get_fun_without_pessimise :
  Provider_context.t -> string -> Typing_defs.fun_elt option

val get_typedef_WARNING_ONLY_FOR_SHMEM :
  Provider_context.t -> string -> Typing_defs.typedef_type option

val get_typedef_without_pessimise :
  Provider_context.t -> string -> Typing_defs.typedef_type option

val get_gconst : Provider_context.t -> string -> Typing_defs.const_decl option

val get_module :
  Provider_context.t -> string -> Typing_defs.module_def_type option
