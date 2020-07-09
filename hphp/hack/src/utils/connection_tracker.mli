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

val create : unit -> t

val log_id : t -> string
