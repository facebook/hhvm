(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val pp : Format.formatter -> t -> unit

val equal : t -> t -> bool

val compare : t -> t -> int

val alloc : unit -> t
