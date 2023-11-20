(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type type_key = string

type typedef_decl = Typing_defs.typedef_type

val get_typedef_WARNING_ONLY_FOR_SHMEM :
  Provider_context.t -> type_key -> typedef_decl option

val find_in_direct_decl_parse :
  cache_results:bool ->
  Provider_context.t ->
  Relative_path.t ->
  type_key ->
  (Shallow_decl_defs.decl -> 'a option) ->
  'a option
