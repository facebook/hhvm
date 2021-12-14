(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'a t = 'a * string [@@deriving eq, ord, show]

val get_message_pos : 'a t -> 'a

val get_message_str : 'a t -> string
