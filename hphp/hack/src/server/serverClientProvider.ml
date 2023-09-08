(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes

exception Client_went_away

type t = {
  default_in_fd: Unix.file_descr;
      (** Default pipe. Used for non time sensitive commands and IDE connections. *)
  priority_in_fd: Unix.file_descr;
      (** Priority pipe.
          There's a handler for events happening on this pipe which will interrupt the typing service.
          In practice this is used for commands that can be served immediately. *)
  force_dormant_start_only_in_fd: Unix.file_descr;
      (** Force formant start only pipe, used to perform the --force-dormant-start command. *)
}

type priority =
  | Priority_high
  | Priority_default
  | Priority_dormant

type persistent_client = {
  fd: Unix.file_descr;
  mutable tracker: Connection_tracker.t;
}

type client =
  | Non_persistent_client of {
      ic: Timeout.in_channel;
      oc: Out_channel.t;
      priority: priority;
      mutable tracker: Connection_tracker.t;
    }
      (** In practice this is hh_client. There can be multiple non-persistent clients. *)
  | Persistent_client of persistent_client
      (** In practice this is the IDE. There is only one persistent client. *)
[@@ocaml.warning "-37"]

type handoff = {
  client: client;
  m2s_sequence_number: int;
      (** A unique number incremented for each client socket handoff from monitor to server.
          Useful to correlate monitor and server logs. *)
}

type select_outcome =
  | Select_persistent
  | Select_new of handoff
  | Select_nothing
  | Select_exception of Exception.t
  | Not_selecting_hg_updating

let provider_from_file_descriptors
    (default_in_fd, priority_in_fd, force_dormant_start_only_in_fd) =
  { default_in_fd; priority_in_fd; force_dormant_start_only_in_fd }

let provider_for_test () = failwith "for use in tests only"

(** Retrieve channels to client from monitor process. *)
let accept_client
    (priority : priority)
    (parent_in_fd : Unix.file_descr)
    (t_sleep_and_check : float)
    (t_monitor_fd_ready : float) : handoff =
  let ({ MonitorRpc.m2s_tracker = tracker; m2s_sequence_number }
        : MonitorRpc.monitor_to_server_handoff_msg) =
    Marshal_tools.from_fd_with_preamble parent_in_fd
  in
  let t_got_tracker = Unix.gettimeofday () in
  Hh_logger.log
    "[%s] got tracker #%d handoff from monitor"
    (Connection_tracker.log_id tracker)
    m2s_sequence_number;
  let socket = Libancillary.ancil_recv_fd parent_in_fd in
  let t_got_client_fd = Unix.gettimeofday () in
  MonitorRpc.write_server_receipt_to_monitor_file
    ~server_receipt_to_monitor_file:
      (ServerFiles.server_receipt_to_monitor_file (Unix.getpid ()))
    ~sequence_number_high_water_mark:m2s_sequence_number;
  Hh_logger.log
    "[%s] got FD#%d handoff from monitor"
    (Connection_tracker.log_id tracker)
    m2s_sequence_number;
  let tracker =
    let open Connection_tracker in
    tracker
    |> track ~key:Server_sleep_and_check ~time:t_sleep_and_check
    |> track ~key:Server_monitor_fd_ready ~time:t_monitor_fd_ready
    |> track ~key:Server_got_tracker ~time:t_got_tracker
    |> track ~key:Server_got_client_fd ~time:t_got_client_fd
  in
  {
    client =
      Non_persistent_client
        {
          ic = Timeout.in_channel_of_descr socket;
          oc = Unix.out_channel_of_descr socket;
          priority;
          tracker;
        };
    m2s_sequence_number;
  }

let select ~idle_gc_slice fd_list timeout =
  let deadline = Unix.gettimeofday () +. timeout in
  match ServerIdleGc.select ~slice:idle_gc_slice ~timeout fd_list with
  | [] ->
    let timeout = Float.(max 0.0 (deadline -. Unix.gettimeofday ())) in
    let (ready_fds, _, _) = Unix.select fd_list [] [] timeout in
    ready_fds
  | ready_fds -> ready_fds

(** Waits up to 0.1 seconds and checks for new connection attempts.
    Select what client to serve next and call
    retrieve channels to client from monitor process. *)
let sleep_and_check
    ({ default_in_fd; priority_in_fd; force_dormant_start_only_in_fd } : t)
    (persistent_client_opt : client option)
    ~(ide_idle : bool)
    ~(idle_gc_slice : int)
    (kind : [< `Any | `Force_dormant_start_only | `Priority ]) : select_outcome
    =
  let t_sleep_and_check = Unix.gettimeofday () in
  let in_fds =
    [default_in_fd; priority_in_fd; force_dormant_start_only_in_fd]
  in
  let fd_l =
    match (kind, persistent_client_opt) with
    | (`Force_dormant_start_only, _) -> [force_dormant_start_only_in_fd]
    | (`Priority, _) -> [priority_in_fd]
    | (`Any, Some (Persistent_client { fd; _ })) ->
      (* If we are not sure that there are no more IDE commands, do not even
       * look at non-persistent client to avoid race conditions. *)
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
  let ready_fd_l = select ~idle_gc_slice fd_l 0.1 in
  let t_monitor_fd_ready = Unix.gettimeofday () in
  (* Prioritize existing persistent client requests over command line ones *)
  let is_persistent fd =
    match persistent_client_opt with
    | Some (Persistent_client { fd = x; _ }) -> Poly.equal fd x
    | _ -> false
  in
  try
    if List.exists ready_fd_l ~f:is_persistent then
      Select_persistent
    else if List.mem ~equal:Poly.( = ) ready_fd_l priority_in_fd then
      Select_new
        (accept_client
           Priority_high
           priority_in_fd
           t_sleep_and_check
           t_monitor_fd_ready)
    else if List.mem ~equal:Poly.( = ) ready_fd_l default_in_fd then
      Select_new
        (accept_client
           Priority_default
           default_in_fd
           t_sleep_and_check
           t_monitor_fd_ready)
    else if List.mem ~equal:Poly.( = ) ready_fd_l force_dormant_start_only_in_fd
    then
      Select_new
        (accept_client
           Priority_dormant
           force_dormant_start_only_in_fd
           t_sleep_and_check
           t_monitor_fd_ready)
    else if List.is_empty ready_fd_l then
      Select_nothing
    else
      failwith "sleep_and_check got impossible fd"
  with
  | End_of_file as exn ->
    let e = Exception.wrap exn in
    HackEventLogger.get_client_channels_exception e;
    Hh_logger.log "GET_CLIENT_CHANNELS_EXCEPTION End_of_file. Terminating.";
    Exit.exit Exit_status.Server_got_eof_from_monitor
  | exn ->
    let e = Exception.wrap exn in
    HackEventLogger.get_client_channels_exception e;
    Hh_logger.log
      "GET_CLIENT_CHANNELS_EXCEPTION(%s). Ignoring."
      (Exception.get_ctor_string e);
    Unix.sleepf 0.5;
    Select_exception e

let has_persistent_connection_request = function
  | Persistent_client { fd; _ } ->
    let (ready, _, _) = Unix.select [fd] [] [] 0.0 in
    not (List.is_empty ready)
  | _ -> false

let priority_fd { priority_in_fd; _ } = Some priority_in_fd

let get_client_fd = function
  | Persistent_client { fd; _ } -> Some fd
  | Non_persistent_client _ -> failwith "not implemented"

let track ~key ?time ?log ?msg ?long_delay_okay client =
  match client with
  | Persistent_client client ->
    client.tracker <-
      Connection_tracker.track
        client.tracker
        ~key
        ?time
        ?log
        ?msg
        ?long_delay_okay
  | Non_persistent_client client ->
    client.tracker <-
      Connection_tracker.track
        client.tracker
        ~key
        ?time
        ?log
        ?msg
        ?long_delay_okay

let say_hello oc =
  let fd = Unix.descr_of_out_channel oc in
  let (_ : int) =
    Marshal_tools.to_fd_with_preamble fd ServerCommandTypes.Hello
  in
  ()

let read_connection_type_from_channel (ic : Timeout.in_channel) :
    connection_type =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> raise Read_command_timeout)
    ~do_:(fun timeout ->
      let connection_type : connection_type = Timeout.input_value ~timeout ic in
      connection_type)

(* Warning 52 warns about using Sys_error. Here we have no alternative but to depend on Sys_error strings *)
[@@@warning "-52"]

let read_connection_type (client : client) : connection_type =
  match client with
  | Non_persistent_client client -> begin
    try
      say_hello client.oc;
      client.tracker <-
        Connection_tracker.(track client.tracker ~key:Server_sent_hello);
      let connection_type : connection_type =
        read_connection_type_from_channel client.ic
      in
      client.tracker <-
        Connection_tracker.(
          track client.tracker ~key:Server_got_connection_type);
      connection_type
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

(* CARE! scope of warning suppression should be only read_connection_type *)
[@@@warning "+52"]

let send_response_to_client client response =
  let (fd, tracker) =
    match client with
    | Non_persistent_client { oc; tracker; _ } ->
      (Unix.descr_of_out_channel oc, tracker)
    | Persistent_client { fd; tracker; _ } -> (fd, tracker)
  in
  let (_ : int) =
    Marshal_tools.to_fd_with_preamble
      fd
      (ServerCommandTypes.Response (response, tracker))
  in
  ()

let read_client_msg ic =
  try
    Timeout.with_timeout
      ~timeout:1
      ~on_timeout:(fun _ -> raise Read_command_timeout)
      ~do_:(fun timeout -> Timeout.input_value ~timeout ic)
  with
  | End_of_file -> raise Client_went_away

let client_has_message = function
  | Non_persistent_client _ -> true
  | Persistent_client { fd; _ } ->
    let (ready, _, _) = Unix.select [fd] [] [] 0.0 in
    not (List.is_empty ready)

let read_client_msg = function
  | Non_persistent_client { ic; _ } -> read_client_msg ic
  | Persistent_client { fd; _ } ->
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
    | Persistent_client { fd; _ } ->
      (Timeout.in_channel_of_descr fd, Unix.out_channel_of_descr fd)
  in
  ServerUtils.shutdown_client (ic, oc)

let ping = function
  | Non_persistent_client { oc; _ } ->
    let fd = Unix.descr_of_out_channel oc in
    let (_ : int) =
      try Marshal_tools.to_fd_with_preamble fd ServerCommandTypes.Ping with
      | _ -> raise Client_went_away
    in
    ()
  | Persistent_client _ -> ()
