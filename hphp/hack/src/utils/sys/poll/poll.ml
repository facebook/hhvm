(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module IPoll = Iomux.Poll

module Flags = struct
  type t =
    | Pollerr
    | Pollnval
    | Pollhup
    | Pollin
    | Pollout
    | Pollpri

  let flags (flags : IPoll.Flags.t) : t list =
    let res = [] in
    let res =
      if IPoll.Flags.mem flags IPoll.Flags.pollerr then
        Pollerr :: res
      else
        res
    in
    let res =
      if IPoll.Flags.mem flags IPoll.Flags.pollnval then
        Pollnval :: res
      else
        res
    in
    let res =
      if IPoll.Flags.mem flags IPoll.Flags.pollhup then
        Pollhup :: res
      else
        res
    in
    let res =
      if IPoll.Flags.mem flags IPoll.Flags.pollin then
        Pollin :: res
      else
        res
    in
    let res =
      if IPoll.Flags.mem flags IPoll.Flags.pollout then
        Pollout :: res
      else
        res
    in
    let res =
      if IPoll.Flags.mem flags IPoll.Flags.pollpri then
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

let make_poll_timeout_ms (t : int option) : IPoll.poll_timeout =
  match t with
  | Some t -> IPoll.Milliseconds t
  | None -> IPoll.Infinite

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
    (flags : IPoll.Flags.t) (fd : Unix.file_descr) ~(timeout_ms : int option) :
    (outcome, Flags.t list) result =
  let timeout = make_poll_timeout_ms timeout_ms in
  let poll = IPoll.create ~maxfds:1 () in
  IPoll.set_index poll 0 fd flags;
  let nready = IPoll.poll poll 1 timeout in
  if nready = 0 then
    Ok Timeout
  else
    let rflags = IPoll.get_revents poll 0 in
    let pollhup = IPoll.Flags.mem rflags IPoll.Flags.pollhup in
    let is_out = IPoll.Flags.mem flags IPoll.Flags.pollout in
    let pollerr = IPoll.Flags.mem rflags IPoll.Flags.pollerr in
    let pollpri = IPoll.Flags.mem rflags IPoll.Flags.pollpri in
    let pollnval = IPoll.Flags.mem rflags IPoll.Flags.pollnval in
    let hup =
      pollhup
      || (* According to the poll man page, POLLERR
            "is also set for a file descriptor
            referring to the write end of a pipe when the read end has
            been closed." *)
      (pollerr && is_out)
    in
    let has_errors = pollpri || pollnval || (pollerr && not is_out) in
    if has_errors then
      Error (Flags.flags rflags)
    else
      Ok (Event { hup; ready = IPoll.Flags.mem rflags flags })

let rec f_non_interrupted f x y ~timeout_ms =
  let start_time = Unix.gettimeofday () in
  try f x y ~timeout_ms with
  | Unix.Unix_error (Unix.EINTR, _, _) ->
    let timeout_ms =
      Option.map timeout_ms ~f:(fun timeout_ms ->
          let elapsed_ms =
            (Unix.gettimeofday () -. start_time) *. 1000. |> Int.of_float
          in
          timeout_ms - elapsed_ms)
    in
    f_non_interrupted f x y ~timeout_ms

let wait_fd_non_intr = f_non_interrupted wait_fd

let wait_fd_read = wait_fd IPoll.Flags.pollin

let wait_fd_read_non_interrupted = wait_fd_non_intr IPoll.Flags.pollin

let wait_fd_write = wait_fd IPoll.Flags.pollout

let wait_fd_write_non_interrupted = wait_fd_non_intr IPoll.Flags.pollout

let ready_fds flags fds ~timeout_ms =
  let timeout = make_poll_timeout_ms timeout_ms in
  let nfds = List.length fds in
  let poll = IPoll.create ~maxfds:nfds () in
  List.iteri fds ~f:(fun idx fd -> IPoll.set_index poll idx fd flags);
  let nready = IPoll.poll poll nfds timeout in
  let ready_fds = ref [] in
  IPoll.iter_ready poll nready (fun _idx fd _revents ->
      ready_fds := fd :: !ready_fds);
  !ready_fds

let ready_fds_non_intr = f_non_interrupted ready_fds

let ready_fds_read_non_interrupted = ready_fds_non_intr IPoll.Flags.pollin

let ready_fds_write_non_interrupted = ready_fds_non_intr IPoll.Flags.pollout
