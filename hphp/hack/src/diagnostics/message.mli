(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'a t = 'a * string [@@deriving eq, hash, ord, show]

val map : 'a t -> f:('a -> 'b) -> 'b t

val get_message_pos : 'a t -> 'a

val get_message_str : 'a t -> string
