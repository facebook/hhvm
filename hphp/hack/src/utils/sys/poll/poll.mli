(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Flags : sig
  type t =
    | Pollerr
    | Pollnval
    | Pollhup
    | Pollin
    | Pollout
    | Pollpri

  val to_string : t list -> string
end

exception Poll_exception of Flags.t list

type outcome =
  | Timeout
  | Event of {
      ready: bool;
          (** The FD is ready to read or write. This may be false
              if e.g. the peer has hung up already and there is nothing to read/write *)
      hup: bool;
          (** If the FD is a pipe or socket, `hup` indicates whether
              the peer has hung up, i.e. closed its end of the channel. *)
    }

(** Wait for a file descriptor to be ready for reading.
      Returns true if ready, false if it timed out. *)
val wait_fd_read :
  Unix.file_descr -> timeout_ms:int option -> (outcome, Flags.t list) result

(** Wait for a file descriptor to be ready for reading.
      Returns true if ready, false if it timed out.

  Ignores EINTR, i.e. retries with an adjusted timeout upon EINTR.
  We implement timers using sigalarm which means this call can be
  interrupted. *)
val wait_fd_read_non_intr :
  Unix.file_descr -> timeout_ms:int option -> (outcome, Flags.t list) result

(** Wait for a file descriptor to be ready for writing.
      Returns true if ready, false if it timed out. *)
val wait_fd_write :
  Unix.file_descr -> timeout_ms:int option -> (outcome, Flags.t list) result

(** Wait for a file descriptor to be ready for writing.
      Returns true if ready, false if it timed out.

  Ignores EINTR, i.e. retries with an adjusted timeout upon EINTR.
  We implement timers using sigalarm which means this call can be
  interrupted. *)
val wait_fd_write_non_intr :
  Unix.file_descr -> timeout_ms:int option -> (outcome, Flags.t list) result
