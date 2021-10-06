(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type module_ [@@deriving eq, show]

val of_string : string -> module_

val of_maybe_string : string option -> module_ option

type t = module_ option [@@deriving eq, show]

(** [can_acesss env ~current ~target] returns whether a symbol defined in
  * module [current] is allowed to access an internal symbol defined in
  * [target] under [env].
  *)
val can_access :
  current:t ->
  target:t ->
  [ `Yes | `Disjoint of string * string | `Outside of string ]
