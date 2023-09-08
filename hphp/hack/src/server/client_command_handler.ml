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
      Hh_logger.log
        "Client went away or server took too long to read command. Shutting down client socket.";
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
      Hh_logger.log "Client channel went bad. Shutting down client socket";
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
    | ServerCommandTypes.Non_persistent ->
      Hh_logger.log "Handling non-persistent client command.";
      ServerCommand.handle genv env client

  let handle_client_command_or_persistent_connection genv env client =
    handle_client_command_or_persistent_connection_ genv env client
    (* Similarly to persistent client commands, we wrap in handle_client_command_try a second time here. *)
    |> ServerUtils.wrap ~try_:(handle_client_command_try (fun x -> x) client)
end

let handle_client_command_or_persistent_connection genv env client client_kind :
    ServerEnv.env ServerUtils.handle_command_result =
  ServerIdle.stamp_connection ();
  match client_kind with
  | `Non_persistent ->
    Hh_logger.log
      ~category:"clients"
      "Handling non-persistent client command or persistent client connection.";
    NonPersistent.handle_client_command_or_persistent_connection genv env client
