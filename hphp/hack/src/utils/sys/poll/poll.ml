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

  let error_flags (flags : Poll.Flags.t) : t list =
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
    res

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

let wait_fd
    (flags : Poll.Flags.t) (fd : Unix.file_descr) ~(timeout_ms : int option) :
    ([ `Ok | `Timeout ], Flags.t list) result =
  let timeout = make_poll_timeout_ms timeout_ms in
  let poll = Poll.create () in
  Poll.set_index poll 0 fd flags;
  let _nready = Poll.poll poll 1 timeout in
  let rflags = Poll.get_revents poll 0 in
  match Flags.error_flags rflags with
  | [] ->
    Ok
      (if Poll.Flags.mem rflags flags then
        `Ok
      else
        `Timeout)
  | _ -> Error (Flags.flags rflags)

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
