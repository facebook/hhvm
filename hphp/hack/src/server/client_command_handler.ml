(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Warning 52 warns about using Sys_error.
 * We have no alternative but to depend on Sys_error strings though. *)
[@@@warning "-52"]

let shutdown_persistent_client client env =
  ClientProvider.shutdown_client client;
  let env =
    {
      env with
      ServerEnv.pending_command_needs_writes = None;
      persistent_client_pending_command_needs_full_check = None;
    }
  in
  ServerFileSync.clear_sync_data env

module Persistent : sig
  val handle_client_command :
    ServerEnv.genv ->
    ServerEnv.env ->
    ClientProvider.client ->
    ServerEnv.env ServerUtils.handle_command_result
end = struct
  let handle_persistent_client_command_exception
      ~(client : ClientProvider.client) ~(is_fatal : bool) (e : Exception.t) :
      unit =
    let open Marshal_tools in
    let remote_e =
      {
        message = Exception.get_ctor_string e;
        stack = Exception.get_backtrace_string e |> Exception.clean_stack;
      }
    in
    let push =
      if is_fatal then
        ServerCommandTypes.FATAL_EXCEPTION remote_e
      else
        ServerCommandTypes.NONFATAL_EXCEPTION remote_e
    in
    begin
      try ClientProvider.send_push_message_to_client client push with _ -> ()
    end;
    HackEventLogger.handle_persistent_connection_exception "inner" ~is_fatal e;
    Hh_logger.error
      "HANDLE_PERSISTENT_CONNECTION_EXCEPTION(inner) %s"
      (Exception.to_string e);
    ()

  (* Warning 52 warns about using Sys_error.
   * We have no alternative but to depend on Sys_error strings though. *)
  [@@@warning "-52"]

  (* Same as handle_client_command_try, but for persistent clients *)
  let handle_persistent_client_command_try
      (type result)
      ~(return :
         ServerEnv.env ->
         finish_command_handling:_ ->
         needs_writes:string option ->
         result)
      client
      env
      (command : unit -> result) : result =
    try command () with
    (* TODO: Make sure the pipe exception is really about this client. *)
    | Unix.Unix_error (Unix.EPIPE, _, _)
    | Sys_error "Connection reset by peer"
    | Sys_error "Broken pipe"
    | ServerCommandTypes.Read_command_timeout
    | ServerClientProvider.Client_went_away ->
      return
        env
        ~finish_command_handling:(shutdown_persistent_client client)
        ~needs_writes:(Some "Client_went_away")
    | ServerCommand.Nonfatal_rpc_exception (e, env) ->
      handle_persistent_client_command_exception ~client ~is_fatal:false e;
      return env ~finish_command_handling:(fun env -> env) ~needs_writes:None
    | exn ->
      let e = Exception.wrap exn in
      handle_persistent_client_command_exception ~client ~is_fatal:true e;
      let needs_writes = Some (Exception.to_string e) in
      return
        env
        ~finish_command_handling:(shutdown_persistent_client client)
        ~needs_writes

  [@@@warning "+52"]

  (* CARE! scope of suppression should be only handle_persistent_client_command_try *)

  let handle_persistent_client_command_ genv env client :
      ServerEnv.env ServerUtils.handle_command_result =
    let return env ~finish_command_handling ~needs_writes =
      match needs_writes with
      | Some reason ->
        ServerUtils.Needs_writes
          {
            env;
            finish_command_handling;
            recheck_restart_is_needed = true;
            reason;
          }
      | None -> ServerUtils.Done (finish_command_handling env)
    in
    handle_persistent_client_command_try ~return client env @@ fun () ->
    let env = { env with ServerEnv.ide_idle = false } in
    ServerCommand.handle genv env client

  let handle_client_command genv env client =
    (* This "return" is guaranteed to be run as part of the main loop, when workers
     * are not busy, so we can ignore whether it needs writes or not - it's always
     * safe for it to write. *)
    let return env ~finish_command_handling ~needs_writes:_ =
      finish_command_handling env
    in
    handle_persistent_client_command_ genv env client
    (* We wrap in handle_persistent_client_command_try a second time here.
       The first time was within [handle_persistent_client_command_],
       and was in order to handle exceptions when receiving the client message.
       This second time is in order to handle exceptions during
       executing the command and sending the response. *)
    |> ServerUtils.wrap
         ~try_:(handle_persistent_client_command_try ~return client)
end

module NonPersistent : sig
  val handle_client_command_or_persistent_connection :
    ServerEnv.genv ->
    ServerEnv.env ->
    ClientProvider.client ->
    ServerEnv.env ServerUtils.handle_command_result
end = struct
  let handle_client_command_exception
      ~(env : ServerEnv.env) ~(client : ClientProvider.client) (e : Exception.t)
      : ServerEnv.env =
    match Exception.to_exn e with
    | ClientProvider.Client_went_away
    | ServerCommandTypes.Read_command_timeout ->
      ClientProvider.shutdown_client client;
      env
    (* Connection dropped off. Its unforunate that we don't actually know
     * which connection went bad (could be any write to any connection to
     * child processes/daemons), we just assume at this top-level that
     * since it's not caught elsewhere, it's the connection to the client.
     *
     * TODO: Make sure the pipe exception is really about this client.*)
    | Unix.Unix_error (Unix.EPIPE, _, _)
    | Sys_error "Broken pipe"
    | Sys_error "Connection reset by peer" ->
      Hh_logger.log "Client channel went bad. Shutting down client connection";
      ClientProvider.shutdown_client client;
      env
    | exn ->
      let e = Exception.wrap exn in
      HackEventLogger.handle_connection_exception "inner" e;
      Hh_logger.log
        "HANDLE_CONNECTION_EXCEPTION(inner) %s"
        (Exception.to_string e);
      ClientProvider.shutdown_client client;
      env

  [@@@warning "+52"]

  (* CARE! scope of suppression should be only handle_client_command_exception *)

  (* [command] represents a non-persistent command coming from client. If executing [command]
   * throws, we need to dispose of this client (possibly recovering updated
   * environment from Nonfatal_rpc_exception). "return" is a constructor
   * wrapping the return value to make it match return type of [command] *)
  let handle_client_command_try return client env command =
    try command () with
    | ServerCommand.Nonfatal_rpc_exception (e, env) ->
      return (handle_client_command_exception ~env ~client e)
    | exn ->
      let e = Exception.wrap exn in
      return (handle_client_command_exception ~env ~client e)

  let handle_client_command_or_persistent_connection_ genv env client =
    ClientProvider.track
      client
      ~key:Connection_tracker.Server_start_handle_connection;
    handle_client_command_try (fun x -> ServerUtils.Done x) client env
    @@ fun () ->
    match ClientProvider.read_connection_type client with
    | ServerCommandTypes.Persistent ->
      let handle_persistent_client_connection env =
        let env =
          match env.ServerEnv.persistent_client with
          | Some old_client ->
            ClientProvider.send_push_message_to_client
              old_client
              ServerCommandTypes.NEW_CLIENT_CONNECTED;
            shutdown_persistent_client old_client env
          | None -> env
        in
        ClientProvider.track client ~key:Connection_tracker.Server_start_handle;
        ClientProvider.send_response_to_client
          client
          ServerCommandTypes.Connected;
        let env =
          {
            env with
            ServerEnv.persistent_client =
              Some (ClientProvider.make_and_store_persistent client);
          }
        in
        (* If the client connected in the middle of recheck, let them know it's
         * happening. *)
        if ServerEnv.is_full_check_started env.ServerEnv.full_check_status then
          ServerBusyStatus.send
            env
            (ServerCommandTypes.Doing_global_typecheck
               (ServerCheckUtils.global_typecheck_kind genv env));
        env
      in
      if
        Option.is_some env.ServerEnv.persistent_client
        (* Cleaning up after existing client (in shutdown_persistent_client)
         * will attempt to write to shared memory *)
      then
        ServerUtils.Needs_writes
          {
            env;
            finish_command_handling = handle_persistent_client_connection;
            recheck_restart_is_needed = true;
            reason = "Cleaning up persistent client";
          }
      else
        ServerUtils.Done (handle_persistent_client_connection env)
    | ServerCommandTypes.Non_persistent -> ServerCommand.handle genv env client

  let handle_client_command_or_persistent_connection genv env client =
    handle_client_command_or_persistent_connection_ genv env client
    (* Similarly to persistent client commands, we wrap in handle_client_command_try a second time here. *)
    |> ServerUtils.wrap ~try_:(handle_client_command_try (fun x -> x) client)
end

let handle_client_command_or_persistent_connection genv env client client_kind :
    ServerEnv.env ServerUtils.handle_command_result =
  ServerIdle.stamp_connection ();
  match client_kind with
  | `Persistent -> Persistent.handle_client_command genv env client
  | `Non_persistent ->
    NonPersistent.handle_client_command_or_persistent_connection genv env client
