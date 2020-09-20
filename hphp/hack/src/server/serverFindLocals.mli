(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Find references in the current file to the local variable at the given
position. *)
val go :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  char:int ->
  Pos.t list

(** Same as [go], but with an AST provided explicitly. Prefer [go] when
possible to make use of caching. *)
val go_from_ast : ast:Nast.program -> line:int -> char:int -> Pos.t list
