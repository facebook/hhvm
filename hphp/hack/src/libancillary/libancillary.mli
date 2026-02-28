(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Receiving_Fd_Exception of string

(** [ancil_send_fd ~to_:dest_fd fd_to_send] sends [fd_to_send] to the process
  at the other end of [dest_fd].

  Returns 0 for success, -1 on failure.*)
val ancil_send_fd : to_:Unix.file_descr -> Unix.file_descr -> int

(** [ancil_recv_fd from_fd] receives a file descriptor over [from_fd].

  Throws Receiving_Fd_Exception upon error *)
val ancil_recv_fd : Unix.file_descr -> Unix.file_descr
