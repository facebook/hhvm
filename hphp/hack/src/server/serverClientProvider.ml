(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerCommandTypes

exception Client_went_away

(* default pipe, priority pipe, force formant start only pipe *)
type t = Unix.file_descr * Unix.file_descr * Unix.file_descr

type priority =
  | Priority_high
  | Priority_default
  | Priority_dormant

type client =
  | Non_persistent_client of {
      ic: Timeout.in_channel;
      oc: Out_channel.t;
      priority: priority;
    }
  | Persistent_client of Unix.file_descr

type select_outcome =
  | Select_persistent
  | Select_new of client
  | Select_nothing

let provider_from_file_descriptors x = x

let provider_for_test () = failwith "for use in tests only"

(** Retrieve channels to client from monitor process. *)
let accept_client (priority : priority) (parent_in_fd : Unix.file_descr) :
    client =
  let socket = Libancillary.ancil_recv_fd parent_in_fd in
  Non_persistent_client
    {
      ic = Timeout.in_channel_of_descr socket;
      oc = Unix.out_channel_of_descr socket;
      priority;
    }

let select ~idle_gc_slice fd_list timeout =
  let deadline = Unix.gettimeofday () +. timeout in
  match ServerIdleGc.select ~slice:idle_gc_slice ~timeout fd_list with
  | [] ->
    let timeout = max 0.0 (deadline -. Unix.gettimeofday ()) in
    let (ready_fds, _, _) = Unix.select fd_list [] [] timeout in
    ready_fds
  | ready_fds -> ready_fds

(* sleep_and_check: waits up to 0.1 seconds and returns what to read from. *)
let sleep_and_check
    ((default_in_fd, priority_in_fd, force_dormant_start_only) :
      Unix.file_descr * Unix.file_descr * Unix.file_descr)
    (persistent_client_opt : client option)
    ~(ide_idle : bool)
    ~(idle_gc_slice : int)
    (kind : [< `Any | `Force_dormant_start_only | `Priority ]) : select_outcome
    =
  let in_fds = [default_in_fd; priority_in_fd; force_dormant_start_only] in
  let l =
    match (kind, persistent_client_opt) with
    | (`Force_dormant_start_only, _) -> [force_dormant_start_only]
    | (`Priority, _) -> [priority_in_fd]
    | (`Any, Some (Persistent_client fd)) ->
      (* If we are not sure that there are no more IDE commands, do not even
       * look at non-persistent client to avoid race conditions.*)
      if not ide_idle then
        [fd]
      else
        fd :: in_fds
    | (`Any, Some (Non_persistent_client _)) ->
      (* The arguments for "sleep_and_check" are "the source of new clients"
       * and the "client we already store in the env". We only store
       * persistent clients *)
      assert false
    | (`Any, None) -> in_fds
  in
  let ready_fd_l = select ~idle_gc_slice l 0.1 in
  (* Prioritize existing persistent client requests over command line ones *)
  let is_persistent fd = Some (Persistent_client fd) = persistent_client_opt in
  try
    if List.exists ready_fd_l ~f:is_persistent then
      Select_persistent
    else if List.mem ~equal:( = ) ready_fd_l priority_in_fd then
      Select_new (accept_client Priority_high priority_in_fd)
    else if List.mem ~equal:( = ) ready_fd_l default_in_fd then
      Select_new (accept_client Priority_default default_in_fd)
    else if List.mem ~equal:( = ) ready_fd_l force_dormant_start_only then
      Select_new (accept_client Priority_dormant force_dormant_start_only)
    else if List.is_empty ready_fd_l then
      Select_nothing
    else
      failwith "sleep_and_check got impossible fd"
  with e ->
    HackEventLogger.get_client_channels_exception e;
    Hh_logger.log "Getting Client FDs failed. Ignoring.";
    Select_nothing

let has_persistent_connection_request = function
  | Persistent_client fd ->
    let (ready, _, _) = Unix.select [fd] [] [] 0.0 in
    ready <> []
  | _ -> false

let priority_fd (_, x, _) = Some x

let get_client_fd = function
  | Persistent_client fd -> Some fd
  | Non_persistent_client _ -> failwith "not implemented"

let say_hello oc =
  let fd = Unix.descr_of_out_channel oc in
  Marshal_tools.to_fd_with_preamble fd ServerCommandTypes.Hello |> ignore

let read_connection_type ic =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> raise Read_command_timeout)
    ~do_:(fun timeout -> Timeout.input_value ~timeout ic)

[@@@warning "-52"]

(* we have no alternative but to depend on Sys_error strings *)

let read_connection_type = function
  | Non_persistent_client { ic; oc; _ } ->
    begin
      try
        say_hello oc;
        read_connection_type ic
      with
      | Sys_error "Connection reset by peer"
      | Unix.Unix_error (Unix.EPIPE, "write", _) ->
        raise Client_went_away
    end
  | Persistent_client _ ->
    (* Every client starts as Non_persistent_client, and after we read its
     * desired connection type, can be turned into Persistent_client
     * (via make_persistent). *)
    assert false

[@@@warning "+52"]

(* CARE! scope of suppression should be only read_connection_type *)

let send_response_to_client client response t =
  let fd =
    match client with
    | Non_persistent_client { oc; _ } -> Unix.descr_of_out_channel oc
    | Persistent_client fd -> fd
  in
  let (_ : int) =
    Marshal_tools.to_fd_with_preamble
      fd
      (ServerCommandTypes.Response (response, t))
  in
  ()

let send_push_message_to_client client response =
  match client with
  | Non_persistent_client _ ->
    failwith "non-persistent clients don't expect push messages "
  | Persistent_client fd ->
    (try
       let (_ : int) =
         Marshal_tools.to_fd_with_preamble fd (ServerCommandTypes.Push response)
       in
       ()
     with Unix.Unix_error (Unix.EPIPE, "write", "") -> raise Client_went_away)

let read_client_msg ic =
  try
    Timeout.with_timeout
      ~timeout:1
      ~on_timeout:(fun _ -> raise Read_command_timeout)
      ~do_:(fun timeout -> Timeout.input_value ~timeout ic)
  with End_of_file -> raise Client_went_away

let client_has_message = function
  | Non_persistent_client _ -> true
  | Persistent_client fd ->
    let (ready, _, _) = Unix.select [fd] [] [] 0.0 in
    ready <> []

let read_client_msg = function
  | Non_persistent_client { ic; _ } -> read_client_msg ic
  | Persistent_client fd ->
    (* TODO: this is probably wrong, since for persistent client we'll
     * construct a new input channel for each message, while the old one
     * could have already buffered it *)
    let ic = Timeout.in_channel_of_descr fd in
    read_client_msg ic

let get_channels = function
  | Non_persistent_client { ic; oc; _ } -> (ic, oc)
  | Persistent_client _ ->
    (* This function is used to "break" the module abstraction for some things
     * we don't have mocking for yet, like STREAM and DEBUG request types. We
     * have mocking for all the features of persistent clients, so this should
     * never be hit *)
    assert false

let make_persistent = function
  | Non_persistent_client { ic; _ } ->
    Persistent_client (Timeout.descr_of_in_channel ic)
  | Persistent_client _ ->
    (* See comment on read_connection_type. Non_persistent_client can be
     * turned into Persistent_client, but not the other way *)
    assert false

let is_persistent = function
  | Non_persistent_client _ -> false
  | Persistent_client _ -> true

let priority_to_string (client : client) : string =
  match client with
  | Persistent_client _ -> "persistent"
  | Non_persistent_client { priority = Priority_high; _ } -> "high"
  | Non_persistent_client { priority = Priority_default; _ } -> "default"
  | Non_persistent_client { priority = Priority_dormant; _ } -> "dormant"

let shutdown_client client =
  let (ic, oc) =
    match client with
    | Non_persistent_client { ic; oc; _ } -> (ic, oc)
    | Persistent_client fd ->
      (Timeout.in_channel_of_descr fd, Unix.out_channel_of_descr fd)
  in
  ServerUtils.shutdown_client (ic, oc)

let ping = function
  | Non_persistent_client { oc; _ } ->
    let fd = Unix.descr_of_out_channel oc in
    let (_ : int) =
      try Marshal_tools.to_fd_with_preamble fd ServerCommandTypes.Ping
      with _ -> raise Client_went_away
    in
    ()
  | Persistent_client _ -> ()
