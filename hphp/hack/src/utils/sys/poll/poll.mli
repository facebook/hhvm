(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Flags : sig
  type error =
    | Pollerr
    | Pollnval
end

exception Poll_exception of Flags.error list

(** Wait for a file descriptor to be ready for reading.
      Returns true if ready, false if it timed out. *)
val wait_fd_read :
  Unix.file_descr -> timeout_ms:int option -> (bool, Flags.error list) result

(** Wait for a file descriptor to be ready for reading.
      Returns true if ready, false if it timed out.

  Ignores EINTR, i.e. retries with an adjusted timeout upon EINTR.
  We implement timers using sigalarm which means this call can be
  interrupted. *)
val wait_fd_read_non_intr :
  Unix.file_descr -> timeout_ms:int option -> (bool, Flags.error list) result

(** Wait for a file descriptor to be ready for writing.
      Returns true if ready, false if it timed out. *)
val wait_fd_write :
  Unix.file_descr -> timeout_ms:int option -> (bool, Flags.error list) result

(** Wait for a file descriptor to be ready for writing.
      Returns true if ready, false if it timed out.

  Ignores EINTR, i.e. retries with an adjusted timeout upon EINTR.
  We implement timers using sigalarm which means this call can be
  interrupted. *)
val wait_fd_write_non_intr :
  Unix.file_descr -> timeout_ms:int option -> (bool, Flags.error list) result
