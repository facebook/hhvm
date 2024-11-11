(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Poll = Iomux.Poll

module Flags = struct
  type t =
    | Pollerr
    | Pollnval
    | Pollhup
    | Pollin
    | Pollout
    | Pollpri

  let flags (flags : Poll.Flags.t) : t list =
    let res = [] in
    let res =
      if Poll.Flags.mem flags Poll.Flags.pollerr then
        Pollerr :: res
      else
        res
    in
    let res =
      if Poll.Flags.mem flags Poll.Flags.pollnval then
        Pollnval :: res
      else
        res
    in
    let res =
      if Poll.Flags.mem flags Poll.Flags.pollhup then
        Pollhup :: res
      else
        res
    in
    let res =
      if Poll.Flags.mem flags Poll.Flags.pollin then
        Pollin :: res
      else
        res
    in
    let res =
      if Poll.Flags.mem flags Poll.Flags.pollout then
        Pollout :: res
      else
        res
    in
    let res =
      if Poll.Flags.mem flags Poll.Flags.pollpri then
        Pollpri :: res
      else
        res
    in
    res

  let contains_error_flags flags =
    Poll.Flags.mem flags Poll.Flags.(pollerr + pollnval + pollpri)

  let error_to_string = function
    | Pollerr -> "POLLERR"
    | Pollnval -> "POLLNVAL"
    | Pollhup -> "POLLHUP"
    | Pollin -> "POLLIN"
    | Pollout -> "POLLOUT"
    | Pollpri -> "POLLPRI"

  let to_string flags =
    List.map flags ~f:error_to_string |> String.concat ~sep:", "
end

exception Poll_exception of Flags.t list

let make_poll_timeout_ms (t : int option) : Poll.poll_timeout =
  match t with
  | Some t -> Poll.Milliseconds t
  | None -> Poll.Infinite

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

let wait_fd
    (flags : Poll.Flags.t) (fd : Unix.file_descr) ~(timeout_ms : int option) :
    (outcome, Flags.t list) result =
  let timeout = make_poll_timeout_ms timeout_ms in
  let poll = Poll.create ~maxfds:1 () in
  Poll.set_index poll 0 fd flags;
  let nready = Poll.poll poll 1 timeout in
  if nready = 0 then
    Ok Timeout
  else (
    assert (nready = 1);
    let rflags = Poll.get_revents poll 0 in
    if Flags.contains_error_flags rflags then
      Error (Flags.flags rflags)
    else
      Ok
        (Event
           {
             hup = Poll.Flags.mem rflags Poll.Flags.pollhup;
             ready = Poll.Flags.mem rflags flags;
           })
  )

let rec wait_fd_non_intr flags fd ~timeout_ms =
  let start_time = Unix.gettimeofday () in
  try wait_fd flags fd ~timeout_ms with
  | Unix.Unix_error (Unix.EINTR, _, _) ->
    let timeout_ms =
      Option.map timeout_ms ~f:(fun timeout_ms ->
          let elapsed_ms =
            (Unix.gettimeofday () -. start_time) *. 1000. |> Int.of_float
          in
          timeout_ms - elapsed_ms)
    in
    wait_fd_non_intr flags fd ~timeout_ms

let wait_fd_read = wait_fd Poll.Flags.pollin

let wait_fd_read_non_intr = wait_fd_non_intr Poll.Flags.pollin

let wait_fd_write = wait_fd Poll.Flags.pollout

let wait_fd_write_non_intr = wait_fd_non_intr Poll.Flags.pollout
