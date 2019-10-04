(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type rpc_error =
  | Disconnected
  | Malformed of string

let rpc_error_to_verbose_string (err : rpc_error) : string =
  match err with
  | Disconnected -> "Disconnected"
  | Malformed s -> Printf.sprintf "Malformed(%s)" s

let rpc_write (fd : Unix.file_descr) (value : 'a) : (unit, rpc_error) result =
  try
    let _writ = Marshal_tools.to_fd_with_preamble fd value in
    Ok ()
  with
  | Unix.Unix_error (Unix.EPIPE, _, _)
  | Unix.Unix_error (Unix.ECONNRESET, _, _) ->
    Error Disconnected

let rpc_read (fd : Unix.file_descr) : ('a, rpc_error) result =
  try
    let value = Marshal_tools.from_fd_with_preamble fd in
    Ok value
  with
  | End_of_file
  | Unix.Unix_error (Unix.ECONNRESET, _, _) ->
    Error Disconnected

let rpc_close_no_err (fd : Unix.file_descr) : unit =
  (try Unix.shutdown fd Unix.SHUTDOWN_ALL with _ -> ());
  (try Unix.close fd with _ -> ());
  ()

let rpc_write_init_message (fd : Unix.file_descr) (kind : Dispatch.kind) :
    (unit, rpc_error) result =
  Hh_json.(
    let info = Dispatch.find_by_kind kind in
    let name = info.Dispatch.name in
    let json = JSON_Object [("command", JSON_String name)] in
    let value = Hh_json.json_to_multiline json in
    rpc_write fd value)

let rpc_read_init_message (fd : Unix.file_descr) :
    (Dispatch.kind, rpc_error) result =
  let rpc_res = rpc_read fd in
  match rpc_res with
  | Error error -> Error error
  | Ok (value : string) ->
    let json_res =
      try Ok (Hh_json.json_of_string value)
      with Hh_json.Syntax_error s -> Error (Malformed s)
    in
    (match json_res with
    | Error error -> Error error
    | Ok json ->
      let command_opt =
        Hh_json_helpers.Jget.string_opt (Some json) "command"
      in
      (match command_opt with
      | None -> Error (Malformed (Printf.sprintf "No 'command' in %s" value))
      | Some command ->
        let info_opt = Dispatch.find_by_name command in
        (match info_opt with
        | None ->
          Error (Malformed (Printf.sprintf "Unknown command '%s'" command))
        | Some info -> Ok info.Dispatch.kind)))

let rpc_request_new_worker (root : string) (kind : Dispatch.kind) :
    (Unix.file_descr, rpc_error) result =
  let sockaddr = Unix.ADDR_UNIX (Args.prototype_sock_file root) in
  let connect_res =
    try Ok (Timeout.open_connection sockaddr) with
    | Unix.Unix_error (Unix.ECONNREFUSED, _, _) (* socket isn't open *)
    | _ ->
      Error Disconnected
  in
  match connect_res with
  | Error error -> Error error
  | Ok (ic, _oc) ->
    (* ic and oc have the same underlying Unix.file_descr *)
    let fd = Timeout.descr_of_in_channel ic in
    let rpc_res = rpc_write_init_message fd kind in
    (match rpc_res with
    | Error error -> Error error
    | Ok () -> Ok fd)

let run () : unit =
  (* Parse command-line arguments *)
  let root = ref "" in
  let options = [Args.root root] in
  let usage = Printf.sprintf "Usage: %s prototype --root ..." Sys.argv.(0) in
  Arg.parse options (Args.only "prototype") usage;

  (* TODO: handle exceptions in argument parsing *)

  (* Check lock-file sentinel+socket. *)
  if not (Lock.grab (Args.prototype_lock_file !root)) then (
    Printf.eprintf "hh_mapreduce prototype - already running\n%!";
    exit 2
  );
  let () = (try Unix.unlink (Args.prototype_sock_file !root) with _ -> ()) in
  let socket = Unix.socket ~cloexec:true Unix.PF_UNIX Unix.SOCK_STREAM 0 in
  Unix.setsockopt socket Unix.SO_REUSEADDR true;
  Unix.bind socket (Unix.ADDR_UNIX (Args.prototype_sock_file !root));
  Unix.listen socket 10;

  Printf.printf "0x4000\n%!";

  (* Loop: fork upon client requests; die upon stdin *)
  let rec loop () : unit =
    match Unix.select [socket; Unix.stdin] [] [] (-1.) with
    | (ready :: _, _, _) when ready = socket ->
      let (fd, _) = Unix.accept socket in
      let fork = Fork.fork () in
      if fork <> 0 then (
        (* parent(prototype) process *)
        (try Unix.close fd with _ -> ());
        loop ()
      ) else (
        (* child(worker) process *)
        (try Unix.close socket with _ -> ());
        let kind = rpc_read_init_message fd in
        match kind with
        | Error err ->
          Printf.eprintf
            "error reading init response from worker - %s"
            (rpc_error_to_verbose_string err);
          exit 1
        | Ok kind ->
          (try
             let info = Dispatch.find_by_kind kind in
             info.Dispatch.run_worker fd;
             exit 0
             (* we won't even bother closing 'fd' *)
           with exn ->
             let exn = Exception.wrap exn in
             Printf.eprintf "error - %s\n%!" (Exception.to_string exn);
             exit 1)
      )
    | _ ->
      (* we won't even bother closing 'socket' *)
      exit 0
  in
  loop ()
