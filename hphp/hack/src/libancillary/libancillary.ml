(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let int_to_fd (i : int) : Unix.file_descr = Obj.magic i

exception Receiving_Fd_Exception of string

external ancil_send_fd_ : Unix.file_descr -> Unix.file_descr -> int
  = "stub_ancil_send_fd"

let ancil_send_fd ~to_:dest_fd fd = ancil_send_fd_ dest_fd fd

external ancil_recv_fd_ :
  Unix.file_descr ->
  int (* The fd received *) * string (* error message on error *)
  = "stub_ancil_recv_fd"

(** Receives a file descriptor from socket_fd. Throws exception on error. *)
let ancil_recv_fd socket_fd =
  let (fd, errmsg) = ancil_recv_fd_ socket_fd in
  if fd = -1 then
    raise (Receiving_Fd_Exception errmsg)
  else
    int_to_fd fd
