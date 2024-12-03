(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [init_unix_socket path] creates a TCP Unix named socket at [path]
  that can accept connections (i.e. it calls `listen` on the socket,
  so that `accept` can subsequently be called). *)
val init_unix_socket : string -> Unix.file_descr

val get_path : string -> string
