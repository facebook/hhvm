(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type 'a t = {
  under_normal_assumptions: 'a;
  under_dynamic_assumptions: 'a option;
}
[@@deriving eq, hash, show]

val mk_without_dynamic : 'a -> 'a t

val map : f:('a -> 'b) -> 'a t -> 'b t

val combine : f:('a -> 'a -> 'a) -> 'a t -> 'a t -> 'a t

val all : 'a t -> 'a list

val cons : 'a t -> 'a list t -> 'a list t

val append : 'a list t -> 'a list t -> 'a list t

val collect : 'a t list -> 'a list t
