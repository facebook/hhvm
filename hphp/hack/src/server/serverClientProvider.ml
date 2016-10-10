(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ServerCommandTypes

exception Client_went_away

type t = Unix.file_descr
type client =
  | Non_persistent_client of Timeout.in_channel * out_channel
  | Persistent_client of Unix.file_descr

let provider_from_file_descriptor x = x
let provider_for_test () = failwith "for use in tests only"

(** Retrieve channels to client from monitor process. *)
let accept_client parent_in_fd =
  let socket = Libancillary.ancil_recv_fd parent_in_fd in
  Non_persistent_client (
    (Timeout.in_channel_of_descr socket), (Unix.out_channel_of_descr socket)
  )

let accept_client_opt parent_in_fd =
  try Some (accept_client parent_in_fd) with
  | e -> begin
    HackEventLogger.get_client_channels_exception e;
    Hh_logger.log "Getting Client FDs failed. Ignoring.";
    None
  end

let sleep_and_check in_fd persistent_client_opt =
  let l = match persistent_client_opt with
    | Some (Persistent_client fd) -> [in_fd ; fd]
    | Some (Non_persistent_client _) ->
        (* The arguments for "sleep_and_check" are "the source of new clients"
         * and the "client we already store in the env". We only store
         * persistent clients *)
        assert false
    | None -> [in_fd]
  in
  let ready_fd_l, _, _ = Unix.select l [] [] (0.5) in
  match ready_fd_l with
    | [_; _] -> accept_client_opt in_fd, true
    | [fd] when fd = in_fd -> accept_client_opt in_fd, false
    | [fd] when fd <> in_fd -> None, true
    | _ -> None, false

let say_hello oc =
  let fd = Unix.descr_of_out_channel oc in
  Marshal_tools.to_fd_with_preamble fd "Hello"

let read_connection_type ic =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout: (fun _ -> raise Read_command_timeout)
    ~do_: (fun timeout -> Timeout.input_value ~timeout ic)

let read_connection_type = function
  | Non_persistent_client (ic, oc) ->
    begin try
      say_hello oc;
      read_connection_type ic
    with
    | Sys_error("Connection reset by peer")
    | Unix.Unix_error(Unix.EPIPE, "write", _) ->
      raise Client_went_away
    end
  | Persistent_client _ ->
    (* Every client starts as Non_persistent_client, and after we read its
     * desired connection type, can be turned into Persistent_client
     * (via make_persistent). *)
    assert false

let send_response_to_client client response =
  match client with
  | Non_persistent_client (_, oc) ->
    let fd = Unix.descr_of_out_channel oc in
    Marshal_tools.to_fd_with_preamble fd response
  | Persistent_client fd ->
    Marshal_tools.to_fd_with_preamble fd (ServerCommandTypes.Response response)

let send_push_message_to_client client response =
  match client with
  | Non_persistent_client _ ->
    failwith "non-persistent clients don't expect push messages "
  | Persistent_client fd ->
    Marshal_tools.to_fd_with_preamble fd (ServerCommandTypes.Push response)


let read_client_msg ic =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout: (fun _ -> raise Read_command_timeout)
    ~do_: (fun timeout -> Timeout.input_value ~timeout ic)

let read_client_msg = function
  | Non_persistent_client (ic, _) -> read_client_msg ic
  | Persistent_client fd ->
    (* TODO: this is probably wrong, since for persistent client we'll
     * construct a new input channel for each message, while the old one
     * could have already buffered it *)
    let ic = Timeout.in_channel_of_descr fd in
    read_client_msg ic

let get_channels = function
  | Non_persistent_client (ic, oc) -> ic, oc
  | Persistent_client _ ->
    (* This function is used to "break" the module abstraction for some things
     * we don't have mocking for yet, like STREAM and DEBUG request types. We
     * have mocking for all the features of persistent clients, so this should
     * never be hit *)
    assert false

let make_persistent = function
  | Non_persistent_client (ic, _) ->
      Persistent_client (Timeout.descr_of_in_channel ic)
  | Persistent_client _ ->
      (* See comment on read_connection_type. Non_persistent_client can be
       * turned into Persistent_client, but not the other way *)
      assert false

let is_persistent = function
  | Non_persistent_client _ -> false
  | Persistent_client _ -> true

let shutdown_client client =
  let ic, oc = match client with
    | Non_persistent_client (ic, oc) -> ic, oc
    | Persistent_client fd ->
        Timeout.in_channel_of_descr fd, Unix.out_channel_of_descr fd
  in
  ServerUtils.shutdown_client (ic, oc)
