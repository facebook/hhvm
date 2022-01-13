(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type module_ [@@deriving eq, show, ord]

val of_string : string -> module_ option

val of_maybe_string : string option -> module_ option

val name_of : module_ -> string

type t = module_ option [@@deriving eq, show]

(** [can_acesss env ~current ~target] returns whether a symbol defined in
  * module [current] is allowed to access an internal symbol defined in
  * [target] under [env].
  *)
val can_access :
  current:Ast_defs.id option ->
  target:Ast_defs.id option ->
  [ `Yes | `Disjoint of Ast_defs.id * Ast_defs.id | `Outside of Ast_defs.id ]
