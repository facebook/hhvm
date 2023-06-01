(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type 'a t =
  | Valid of 'a
  | Invalid of 'a
[@@deriving compare, sexp]

val valid : 'a -> 'a t

val invalid : 'a -> 'a t

include Core.Applicative.S with type 'a t := 'a t
