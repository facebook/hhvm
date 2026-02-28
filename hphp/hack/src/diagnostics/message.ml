(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Base.Export

(* Trick the Rust generators to use a BStr, because some error
 * messages include a Hack string literal, which are not utf8. *)
type t_byte_string = string [@@deriving eq, hash, ord, show]

(** We use `Pos.t message` and `Pos_or_decl.t message` on the server
    and convert to `Pos.absolute message` before sending it to the client *)
type 'a t = 'a * t_byte_string [@@deriving eq, hash, ord, show]

let map (pos, msg) ~f = (f pos, msg)

let get_message_pos (pos, _) = pos

let get_message_str (_, str) = str
