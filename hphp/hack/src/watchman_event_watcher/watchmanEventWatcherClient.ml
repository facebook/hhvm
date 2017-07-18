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
module Responses = WatchmanEventWatcherConfig.Responses

module Client_actual = struct

(**
 * We track the last known state of the Watcher so we can return a response
 * in get_status even when there's nothing to be read from the reader.
 *
 * This let's us differentiate between "The Watcher connection has gone down"
 * (returning None) and "The Watcher has nothing to tell us right now"
 * (returning the last known state).
 *)
type state =
  | Unknown of Buffered_line_reader.t
  | Mid_update of Buffered_line_reader.t
  (** Settled message has been read. No further reads from
   * the File Descriptor are done (the FD is actually dropped entirely). *)
  | Settled
  (** Connection to the Watcher has failed. Cannot read further updates. *)
  | Failed

type t_ = {
  state : state ref;
  root : Path.t
}

type t = t_ option

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
      state = ref @@ Unknown reader;
      root;
    }
  with
  | Unix.Unix_error (Unix.ENOENT, _, _) ->
    None
  | Unix.Unix_error (Unix.ECONNREFUSED, _, _) ->
    None

let get_status_ instance =
  match !(instance.state) with
  | Failed ->
    None
  | Settled ->
    Some Responses.Settled
  | Mid_update reader
  | Unknown reader when Buffered_line_reader.is_readable reader ->
    begin try
      let msg = Buffered_line_reader.get_next_line ~approx_size:25 reader in
      let msg = Responses.of_string msg in
      let response = begin match msg with
      | Responses.Unknown ->
        Responses.Unknown
      | Responses.Mid_update ->
        instance.state := Mid_update reader;
        Responses.Mid_update
      | Responses.Settled ->
        instance.state := Settled;
        ignore_unix_error
          Unix.close @@ (Buffered_line_reader.get_fd reader);
        Responses.Settled
      end in
      Some response
    with
    | Unix.Unix_error (Unix.EPIPE, _, _)
    | End_of_file ->
      instance.state := Failed;
      None
    end
  | Mid_update _ ->
    Some Responses.Mid_update
  | Unknown _ ->
    Some Responses.Unknown

    module Mocking = struct
      exception Cannot_set_when_mocks_disabled
      let get_status_returns _ =
        raise Cannot_set_when_mocks_disabled
    end

  let get_status t =
    let (>>=) = Option.(>>=) in
    t >>= get_status_

end

module Client_mock = struct
  type t = string option
  module Mocking = struct
    let status = ref None
    let get_status_returns v =
      status := v
  end

  let init _ = None
  let get_status _ = !(Mocking.status)
end

include (val (if Injector_config.use_test_stubbing
  then (module Client_mock : WatchmanEventWatcherClient_sig.S)
  else (module Client_actual : WatchmanEventWatcherClient_sig.S)
))
