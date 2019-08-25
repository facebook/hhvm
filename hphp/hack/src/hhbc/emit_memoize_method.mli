(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type memoize_info

(* Record everything about a class and its methods that's used to generate
 * wrapper code for the <<__Memoize>> methods *)
val make_info :
  Tast.class_ -> Hhbc_id.Class.t -> Tast.method_ list -> memoize_info

(* Emit wrapper methods for <<__Memoize>> methods in the list *)
val emit_wrapper_methods :
  Emit_env.t ->
  memoize_info ->
  Tast.class_ ->
  Tast.method_ list ->
  Hhas_method.t list
