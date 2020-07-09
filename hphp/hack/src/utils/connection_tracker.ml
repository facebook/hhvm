(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  id: string;
  mutable t_start_server_handle: float;
}

let create () : t =
  { id = Random_id.short_string (); t_start_server_handle = 0. }

let log_id (t : t) : string = "mc#" ^ t.id
