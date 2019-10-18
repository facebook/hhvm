(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

external map_shared_memory_ffi :
  string -> (Decl_service_client.sharedmem_base_address, string) result
  = "map_shared_memory"

type file_descr = Prototype_file_descr of Unix.file_descr

let file_descr (fd : Unix.file_descr) : file_descr = Prototype_file_descr fd

type 'a rpc_payload =
  | Ok_payload of 'a
  | Exception_payload of Marshal_tools.remote_exception_data

let rpc_write (pfd : file_descr) (value : 'a) :
    (unit, Marshal_tools.error) result =
  let (Prototype_file_descr fd) = pfd in
  try
    let _written : int =
      Marshal_tools.to_fd_with_preamble fd (Ok_payload value)
    in
    Ok ()
  with
  | (Unix.Unix_error (Unix.EPIPE, _, _) as e)
  | (Unix.Unix_error (Unix.ECONNRESET, _, _) as e) ->
    Error (Marshal_tools.Rpc_disconnected (Exception.wrap e))

let rpc_read (pfd : file_descr) : ('a, Marshal_tools.error) result =
  let (Prototype_file_descr fd) = pfd in
  try
    match (Marshal_tools.from_fd_with_preamble fd : 'a rpc_payload) with
    | Ok_payload value -> Ok value
    | Exception_payload edata -> Error (Marshal_tools.Rpc_remote_panic edata)
  with
  | (End_of_file as e)
  | (Unix.Unix_error (Unix.ECONNRESET, _, _) as e) ->
    Error (Marshal_tools.Rpc_disconnected (Exception.wrap e))

let rpc_close_no_err (pfd : file_descr) : unit =
  let (Prototype_file_descr fd) = pfd in
  (try Unix.shutdown fd Unix.SHUTDOWN_ALL with _ -> ());
  (try Unix.close fd with _ -> ());
  ()

let rpc_write_init_message (pfd : file_descr) (kind : Dispatch.kind) :
    (unit, Marshal_tools.error) result =
  Hh_json.(
    let info = Dispatch.find_by_kind kind in
    let name = info.Dispatch.name in
    let json = JSON_Object [("command", JSON_String name)] in
    let value = Hh_json.json_to_multiline json in
    rpc_write pfd value)

let rpc_read_init_message (pfd : file_descr) :
    (Dispatch.kind, Marshal_tools.error) result =
  let rpc_res = rpc_read pfd in
  match rpc_res with
  | Error error -> Error error
  | Ok (value : string) ->
    let json_res =
      try Ok (Hh_json.json_of_string value)
      with Hh_json.Syntax_error s ->
        Error
          (Marshal_tools.Rpc_malformed
             (s, Utils.Callstack (Printexc.get_backtrace ())))
    in
    (match json_res with
    | Error error -> Error error
    | Ok json ->
      let command_opt =
        Hh_json_helpers.Jget.string_opt (Some json) "command"
      in
      (match command_opt with
      | None ->
        let stack = Utils.Callstack (Printexc.get_backtrace ()) in
        Error
          (Marshal_tools.Rpc_malformed
             (Printf.sprintf "No 'command' in %s" value, stack))
      | Some command ->
        let info_opt = Dispatch.find_by_name command in
        (match info_opt with
        | None ->
          let stack = Utils.Callstack (Printexc.get_backtrace ()) in
          Error
            (Marshal_tools.Rpc_malformed
               (Printf.sprintf "Unknown command '%s'" command, stack))
        | Some info -> Ok info.Dispatch.kind)))

let rpc_request_new_worker (root : string) (kind : Dispatch.kind) :
    (file_descr, Marshal_tools.error) result =
  let sockaddr = Unix.ADDR_UNIX (Args.prototype_sock_file root) in
  let connect_res =
    try Ok (Timeout.open_connection sockaddr) with
    | Unix.Unix_error (Unix.ECONNREFUSED, _, _) as e ->
      Error (Marshal_tools.Rpc_absent (Exception.wrap e))
    | e -> Error (Marshal_tools.Rpc_disconnected (Exception.wrap e))
  in
  match connect_res with
  | Error error -> Error error
  | Ok (ic, _oc) ->
    (* ic and oc have the same underlying Unix.file_descr *)
    let fd = Timeout.descr_of_in_channel ic in
    let pfd = Prototype_file_descr fd in
    let rpc_res = rpc_write_init_message pfd kind in
    (match rpc_res with
    | Error error -> Error error
    | Ok () -> Ok pfd)

let run () : unit =
  (* Parse command-line arguments *)
  let root = ref "" in
  let decl_socket_file = ref "" in
  let cache_directory = ref "" in
  let options =
    [Args.root root; Args.decl decl_socket_file; Args.cache cache_directory]
  in
  let usage =
    Printf.sprintf
      "Usage: %s prototype --root ... --decl ... --cache ..."
      Sys.argv.(0)
  in
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

  (* Map cachelib shared-memory *)
  let base_addr =
    match map_shared_memory_ffi !cache_directory with
    | Ok base_addr ->
      let base_addr_int = ((Obj.magic base_addr : int) lsl 1) + 1 in
      Printf.printf "Prototype cache base address: 0x%x\n%!" base_addr_int;
      base_addr
    | Error message ->
      Printf.eprintf "Prototype failed to map shared-mem: %s\n%!" message;
      exit 1
  in
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
        let pfd = Prototype_file_descr fd in
        let kind = rpc_read_init_message pfd in
        match kind with
        | Error err ->
          Printf.eprintf
            "error reading init response from worker - %s"
            (Marshal_tools.error_to_verbose_string err);
          exit 1
        | Ok kind ->
          (try
             let info = Dispatch.find_by_kind kind in
             let decl =
               match Decl_service_client.init !decl_socket_file base_addr with
               | Ok decl -> decl
               | Error err ->
                 failwith
                   ( "Worker can't connect to decl service - %s"
                   ^ Marshal_tools.error_to_verbose_string err )
             in
             info.Dispatch.run_worker fd decl;
             exit 0
             (* we won't even bother closing 'fd' *)
           with exn ->
             let stack = Printexc.get_backtrace () in
             let message = Exn.to_string exn in
             let payload =
               Exception_payload { Marshal_tools.message; stack }
             in
             begin
               try
                 let _written : int =
                   Marshal_tools.to_fd_with_preamble fd payload
                 in
                 Printf.eprintf "Panic sent to orchestrator: %s\n" message
               with e ->
                 let e = Exception.wrap e in
                 Printf.eprintf
                   "Panic: %s\n%s\nUnable to send panic to orchestrator: %s"
                   message
                   stack
                   (Exception.to_string e)
             end;
             exit 1)
      )
    | _ ->
      (* we won't even bother closing 'socket' *)
      exit 0
  in
  loop ()
