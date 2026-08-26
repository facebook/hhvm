(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Canonical keys for memoizing shape-splat operands. Ordinary types retain
    their structural identity. Newtypes retain their name, arguments, and
    module opacity, but omit their stored bound because consumers re-fetch it
    from the typedef. Omitting it prevents comparison from branching through
    each level of a deeply nested splat-bound chain. *)

open Typing_defs

type t

val of_ty : locl_ty -> t

val compare : t -> t -> int

module Map : Stdlib.Map.S with type key = t
