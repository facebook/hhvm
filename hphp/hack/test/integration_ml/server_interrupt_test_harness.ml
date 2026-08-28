(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude

type t = {
  root: Path.t;
  hhi: Path.t;
  tmp: Path.t;
}

type errors_stream = {
  fd: Unix.file_descr;
  queue:
    ( Server_progress.ErrorsRead.read_result,
      Server_progress_lwt.watch_error )
    result
    Lwt_stream.t;
}

let hh (env : t) (args : string array) : string Lwt.t =
  Sys.chdir (Path.to_string env.root);
  let hh_client_path = Sys.getenv "HH_CLIENT_PATH" in
  let process_env =
    Array.append
      [| "HH_TMPDIR=" ^ Path.to_string env.tmp |]
      (Unix.environment ())
  in
  let (cancel, canceller) = Lwt.wait () in
  let _ =
    Lwt_unix.sleep 120.0 |> Lwt.map (fun () -> Lwt.wakeup_later canceller ())
  in
  let%lwt result =
    Lwt_utils.exec_checked
      (Exec_command.For_use_in_testing_only hh_client_path)
      ~cancel
      ~env:process_env
      args
  in
  match result with
  | Error error -> Lwt.return (Lwt_utils.Process_failure.to_string error)
  | Ok { Lwt_utils.Process_success.stdout; _ } -> Lwt.return stdout

let with_hh_process
    (env : t) (args : string array) (f : Lwt_process.process -> 'a Lwt.t) :
    'a Lwt.t =
  let hh_client_path = Sys.getenv "HH_CLIENT_PATH" in
  let process_env =
    Array.append
      [| "HH_TMPDIR=" ^ Path.to_string env.tmp |]
      (Unix.environment ())
  in
  let process =
    Lwt_process.open_process
      ~env:process_env
      ~cwd:(Path.to_string env.root)
      (hh_client_path, Array.append [| hh_client_path |] args)
  in
  let%lwt () = Lwt_io.close process#stdin in
  Lwt.finalize
    (fun () -> f process)
    (fun () ->
      process#terminate;
      let%lwt _ = process#status in
      Lwt.return_unit)

let write_files (env : t) (files : (string * string) list) : unit =
  List.iter files ~f:(fun (name, contents) ->
      Sys_utils.write_file
        ~file:(Path.concat env.root name |> Path.to_string)
        contents)

let wait_for_errors_file ~(deadline : float) (errors_file_path : string) :
    Unix.file_descr Lwt.t =
  let rec loop () =
    match
      try Some (Unix.openfile errors_file_path [Unix.O_RDONLY] 0) with
      | Unix.Unix_error (Unix.ENOENT, _, _) -> None
    with
    | Some fd -> Lwt.return fd
    | None when Float.(Unix.gettimeofday () > deadline) ->
      failwith (Printf.sprintf "Timeout waiting for %s" errors_file_path)
    | None ->
      let%lwt () = Lwt_unix.sleep 0.1 in
      loop ()
  in
  loop ()

let with_current_errors_stream (env : t) (f : errors_stream -> 'a Lwt.t) :
    'a Lwt.t =
  let errors_file_path = ServerFiles.errors_file_path env.root in
  let%lwt fd =
    wait_for_errors_file
      errors_file_path
      ~deadline:(Unix.gettimeofday () +. 60.0)
  in
  let { Server_progress.ErrorsRead.pid; _ } =
    Server_progress.ErrorsRead.openfile fd |> Result.ok |> Option.value_exn
  in
  let queue = Server_progress_lwt.watch_errors_file ~pid fd in
  Lwt.finalize
    (fun () -> f { fd; queue })
    (fun () ->
      Unix.close fd;
      Lwt.return_unit)

let wait_for_event
    ?(timeout = 60.0)
    ({ queue; _ } : errors_stream)
    ~(description : string)
    ~(matches : string -> bool) : unit Lwt.t =
  let observations = ref [] in
  let rec loop () =
    let%lwt item = Lwt_stream.get queue in
    match item with
    | None ->
      failwith (Printf.sprintf "Errors stream closed before %s" description)
    | Some item ->
      let observed =
        match item with
        | Error error -> Server_progress_lwt.watch_error_short_description error
        | Ok result -> Server_progress_test_helpers.show_read_result result
      in
      observations := observed :: !observations;
      if matches observed then
        Lwt.return_unit
      else
        loop ()
  in
  Lwt.catch
    (fun () -> Lwt_unix.with_timeout timeout loop)
    (function
      | Lwt_unix.Timeout ->
        failwith
          (Printf.sprintf
             "Timed out waiting for %s. Observed:\n%s"
             description
             (List.rev !observations |> String.concat ~sep:"\n"))
      | exn -> Lwt.fail exn)

let server_log_offset (env : t) : int =
  let path = ServerFiles.log_link env.root in
  try (Unix.stat path).Unix.st_size with
  | Unix.Unix_error (Unix.ENOENT, _, _) -> 0

let server_log_since (env : t) ~(offset : int) : string =
  let contents = Sys_utils.cat (ServerFiles.log_link env.root) in
  String.drop_prefix contents offset

let wait_for_server_log
    ?(timeout = 60.0)
    (env : t)
    ~(offset : int)
    ~(description : string)
    ~(matches : string -> bool) : unit Lwt.t =
  let deadline = Unix.gettimeofday () +. timeout in
  let rec loop () =
    let log = server_log_since env ~offset in
    if matches log then
      Lwt.return_unit
    else if Float.(Unix.gettimeofday () > deadline) then
      failwith
        (Printf.sprintf
           "Timed out waiting for %s. Observed:\n%s"
           description
           log)
    else
      let%lwt () = Lwt_unix.sleep 0.1 in
      loop ()
  in
  loop ()

let with_server
    ~(hhconfig : string) ~(files : (string * string) list) (f : t -> unit Lwt.t)
    : unit Lwt.t =
  let root = Tempfile.mkdtemp ~skip_mocking:true in
  let hhi = Tempfile.mkdtemp ~skip_mocking:true in
  let tmp = Tempfile.mkdtemp ~skip_mocking:true in
  let env = { root; hhi; tmp } in
  Lwt_utils.try_finally
    ~f:(fun () ->
      Sys_utils.write_file
        ~file:(Path.concat root ".hhconfig" |> Path.to_string)
        hhconfig;
      Sys_utils.mkdir_p
        ~skip_mocking:true
        (Path.concat root ".hg" |> Path.to_string);
      write_files env files;
      Server_progress.set_root root;
      ServerFiles.set_tmp_FOR_TESTING_ONLY tmp;
      f env)
    ~finally:(fun () ->
      let%lwt _ = hh env [| "stop" |] in
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string root);
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string hhi);
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string tmp);
      Lwt.return_unit)
