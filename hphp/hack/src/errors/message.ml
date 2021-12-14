(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** We use `Pos.t message` and `Pos_or_decl.t message` on the server
    and convert to `Pos.absolute message` before sending it to the client *)
type 'a t = 'a * string [@@deriving eq, ord, show]

let map (pos, msg) ~f = (f pos, msg)

let get_message_pos (pos, _) = pos

let get_message_str (_, str) = str
