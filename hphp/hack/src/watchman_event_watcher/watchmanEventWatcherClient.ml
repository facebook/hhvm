(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * A client to get the repo state from an already-running
 * server watching a repo.
 *
 * See .mli file for details.
 *)

module Config = WatchmanEventWatcherConfig

type state =
  | Unsettled of Buffered_line_reader.t
  (** Settled message has been read. No further reads from
   * the File Descriptor are done (the FD is actually dropped entirely). *)
  | Settled

type t = {
  state : state ref;
  root : Path.t
}

let ignore_unix_error f x =
  try f x with
  | Unix.Unix_error _ ->
    ()

let init root =
  let socket_file = Config.socket_file root in
  let sock_path = Socket.get_path socket_file in
  (** Copied wholesale from MonitorConnection *)
  let sockaddr =
    if Sys.win32 then
      let ic = open_in_bin sock_path in
      let port = input_binary_int ic in
      close_in ic;
      Unix.(ADDR_INET (inet_addr_loopback, port))
    else
      Unix.ADDR_UNIX sock_path
  in
  try
    let (tic, _) = Timeout.open_connection sockaddr in
    let reader = Buffered_line_reader.create
      @@ Timeout.descr_of_in_channel @@ tic in
    Some {
      state = ref @@ Unsettled reader;
      root;
    }
  with
  | Unix.Unix_error (Unix.ENOENT, _, _) ->
    None
  | Unix.Unix_error (Unix.ECONNREFUSED, _, _) ->
    None
  | Timeout.Timeout ->
    None

let is_settled instance =
  match !(instance.state) with
  | Settled ->
    true
  | Unsettled reader ->
    if Buffered_line_reader.is_readable reader then
      let result = Buffered_line_reader.get_next_line ~approx_size:25 reader in
      let settled = String.equal result Config.settled_str in
      if settled then
        let () = instance.state := Settled in
        let () = ignore_unix_error
          Unix.close @@ ((Buffered_line_reader.get_fd reader)) in
        true
      else
        false
    else
      false
