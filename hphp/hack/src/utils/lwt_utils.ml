(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

exception WrappedException of Exception.t

(** Use this instead of [Lwt_main.run] to ensure that the right engine is set, and to
    improve stack traces. *)
let run_main f =
  (* TODO: Lwt defaults to libev when it's installed. Historically, it wasn't installed
     and so we haven't tested it. Here we explicitly choose to use select instead, but
     should test libev. *)
  Lwt_engine.set (new Lwt_engine.select);
  try
    Lwt_main.run
      (try%lwt f () with
      | exn ->
        (* Lwt_main.run loses backtraces when its callback errors. To work around it, we
           catch exceptions within the callback, store their traces using Exception.wrap,
           then re-raise them outside the Lwt_main.run.

           https://github.com/ocsigen/lwt/issues/720 is tracking a more holistic fix. *)
        let exn = Exception.wrap exn in
        raise (WrappedException exn))
  with
  | WrappedException exn -> Exception.reraise exn

let select
    (read_fds : Unix.file_descr list)
    (write_fds : Unix.file_descr list)
    (exn_fds : Unix.file_descr list)
    (timeout : float) :
    (Unix.file_descr list * Unix.file_descr list * Unix.file_descr list) Lwt.t =
  let make_task
      ~(fds : Unix.file_descr list)
      ~(condition : Lwt_unix.file_descr -> bool)
      ~(wait_f : Lwt_unix.file_descr -> unit Lwt.t) :
      (Unix.file_descr list, Unix.file_descr list) result Lwt.t =
    try%lwt
      let fds = List.map fds ~f:Lwt_unix.of_unix_file_descr in
      let%lwt () = Lwt.pick (List.map fds ~f:wait_f) in
      let actionable_fds =
        fds |> List.filter ~f:condition |> List.map ~f:Lwt_unix.unix_file_descr
      in
      Lwt.return (Ok actionable_fds)
    with
    | _ ->
      (* Although we gather a list of exceptional file descriptors here, it
         happens that no call site of `Unix.select` in the codebase has checked
         this list, so we could in theory just return any list (or not return any
         exceptional file descriptors at all). *)
      let exceptional_fds =
        List.filter exn_fds ~f:(fun fd -> List.mem ~equal:Poly.( = ) fds fd)
      in
      Lwt.return (Error exceptional_fds)
  in
  let tasks = [] in
  (* WRITE TASK *)
  let tasks =
    match write_fds with
    | [] -> tasks
    | _ ->
      let write_task =
        let%lwt writeable_fds =
          make_task
            ~fds:write_fds
            ~condition:Lwt_unix.writable
            ~wait_f:Lwt_unix.wait_write
        in
        match writeable_fds with
        | Ok fds -> Lwt.return ([], fds, [])
        | Error fds -> Lwt.return ([], [], fds)
      in
      write_task :: tasks
  in
  (* READ TASK *)
  let tasks =
    match read_fds with
    | [] -> tasks
    | _ ->
      let read_task =
        let%lwt readable_fds =
          make_task
            ~fds:read_fds
            ~condition:Lwt_unix.readable
            ~wait_f:Lwt_unix.wait_read
        in
        match readable_fds with
        | Ok fds -> Lwt.return (fds, [], [])
        | Error fds -> Lwt.return ([], [], fds)
      in
      read_task :: tasks
  in
  (* TIMEOUT TASK *)
  let tasks =
    if Float.(timeout > 0.0) then
      let timeout_task =
        let%lwt () = Lwt_unix.sleep timeout in
        Lwt.return ([], [], [])
      in
      timeout_task :: tasks
    else
      failwith "Timeout <= 0 not implemented"
  in
  Lwt.pick tasks

module Process_success = struct
  type t = {
    command_line: string;
    stdout: string;
    stderr: string;
    start_time: float;
    end_time: float;
  }
end

module Process_failure = struct
  type t = {
    command_line: string;
    process_status: Unix.process_status;
    stdout: string;
    stderr: string;
    exn: exn option;
    start_time: float;
    end_time: float;
  }

  let to_string (process_failure : t) : string =
    let exn_message =
      match process_failure.exn with
      | Some exn -> Exn.to_string exn
      | None -> "<none>"
    in
    let exit_code =
      match process_failure.process_status with
      | Unix.WEXITED exit_code ->
        Printf.sprintf
          "WEXITED %d (%s)"
          exit_code
          (Exit_status.exit_code_to_string exit_code)
      | Unix.WSIGNALED exit_code ->
        Printf.sprintf
          "WSIGNALLED %d (%s)%s"
          exit_code
          (PrintSignal.string_of_signal exit_code)
          (if exit_code = Sys.sigkill then
            " - this often indicates a timeout"
          else
            "")
      | Unix.WSTOPPED exit_code ->
        Printf.sprintf
          "WSTOPPED %d (%s)"
          exit_code
          (PrintSignal.string_of_signal exit_code)
    in
    let stderr =
      match process_failure.stderr with
      | "" -> "<none>"
      | stderr -> stderr
    in
    Printf.sprintf
      ("Process '%s' failed with\n"
      ^^ "Exit code: %s\n"
      ^^ "%s -- %s\n"
      ^^ "Exception: %s\n"
      ^^ "Stderr: %s\n"
      ^^ "Stdout: %s")
      process_failure.command_line
      exit_code
      (Utils.timestring process_failure.start_time)
      (Utils.timestring process_failure.end_time)
      exn_message
      stderr
      process_failure.stdout
end

let exec_checked
    ?(input : string option)
    ?(env : string array option)
    ?(timeout : float option)
    ?(cancel : unit Lwt.t option)
    (program : Exec_command.t)
    (args : string array) : (Process_success.t, Process_failure.t) Lwt_result.t
    =
  let start_time = Unix.gettimeofday () in
  let command =
    match (program, args) with
    | (Exec_command.Shell, [| script |]) -> Lwt_process.shell script
    | (Exec_command.Shell, _) ->
      failwith "Exec_command.Shell needs exactly one arg"
    | (program, args) ->
      let program = Exec_command.to_string program in
      (program, Array.append [| program |] args)
  in
  let command_line = snd command |> String.concat_array ~sep:" " in
  let process = Lwt_process.open_process_full command ?timeout ?env in
  let cancel_watcher =
    match cancel with
    | None -> []
    | Some cancel ->
      [
        (let%lwt () = cancel in
         process#kill Sys.sigkill;
         Lwt.return_unit);
      ]
  in
  (let%lwt (exn, stdout, stderr) =
     let exn = ref None in
     let stdout = ref "" in
     let stderr = ref "" in
     let%lwt () =
       try%lwt
         let%lwt () =
           match input with
           | Some input ->
             let%lwt () = Lwt_io.write process#stdin input in
             let%lwt () = Lwt_io.close process#stdin in
             Lwt.return_unit
           | None -> Lwt.return_unit
         and () =
           let%lwt result = Lwt_io.read process#stdout in
           stdout := result;
           Lwt.return_unit
         and () =
           let%lwt result = Lwt_io.read process#stderr in
           stderr := result;
           Lwt.return_unit
         and () =
           Lwt.choose
             (cancel_watcher
             @ [
                 (let%lwt _ = process#status in
                  Lwt.return_unit);
               ])
         in
         Lwt.return_unit
       with
       | e ->
         exn := Some e;
         Lwt.return_unit
     in
     Lwt.return (!exn, !stdout, !stderr)
   in
   let%lwt state = process#close in
   let end_time = Unix.gettimeofday () in
   match state with
   | Unix.WEXITED 0 ->
     Lwt.return_ok
       { Process_success.command_line; stdout; stderr; start_time; end_time }
   | process_status ->
     Lwt.return_error
       {
         Process_failure.command_line;
         process_status;
         stdout;
         stderr;
         exn;
         start_time;
         end_time;
       })
    [%finally
      let%lwt (_ : Unix.process_status) = process#close in
      Lwt.return_unit]

let try_finally ~(f : unit -> 'a Lwt.t) ~(finally : unit -> unit Lwt.t) :
    'a Lwt.t =
  let%lwt res =
    try%lwt
      let%lwt result = f () in
      Lwt.return result
    with
    | exn ->
      let e = Exception.wrap exn in
      let%lwt () = finally () in
      Exception.reraise e
  in
  let%lwt () = finally () in
  Lwt.return res

let with_lock
    (fd : Lwt_unix.file_descr)
    (lock_command : Unix.lock_command)
    ~(f : unit -> 'a Lwt.t) : 'a Lwt.t =
  (* helper function to set current file position to 0, then do "f",
     then restore it to what it was before *)
  let with_pos0 ~f =
    let%lwt pos = Lwt_unix.lseek fd 0 Unix.SEEK_CUR in
    let%lwt _ = Lwt_unix.lseek fd 0 Unix.SEEK_SET in
    let%lwt result = f () in
    let%lwt _ = Lwt_unix.lseek fd pos Unix.SEEK_SET in
    Lwt.return result
  in

  try_finally
    ~f:(fun () ->
      (* lockf is applied starting at the file-descriptors current position.
         We use "with_pos0" so that when we acquire or release the lock,
         we're locking from the start of the file through to (len=0) the end. *)
      let%lwt () = with_pos0 ~f:(fun () -> Lwt_unix.lockf fd lock_command 0) in
      let%lwt r = f () in
      Lwt.return r)
    ~finally:(fun () ->
      let%lwt () = with_pos0 ~f:(fun () -> Lwt_unix.lockf fd Unix.F_ULOCK 0) in
      Lwt.return_unit)

let with_context
    ~(enter : unit -> unit Lwt.t)
    ~(exit : unit -> unit Lwt.t)
    ~(do_ : unit -> 'a Lwt.t) : 'a Lwt.t =
  let%lwt () = enter () in
  try_finally ~f:do_ ~finally:exit

let read_all (path : string) : (string, string) Lwt_result.t =
  try%lwt
    let%lwt contents =
      Lwt_io.with_file ~mode:Lwt_io.Input path (fun ic ->
          let%lwt contents = Lwt_io.read ic in
          Lwt.return contents)
    in
    Lwt.return (Ok contents)
  with
  | _ ->
    Lwt.return
      (Error
         (Printf.sprintf
            "Could not read the contents of the file at path %s"
            path))

module Promise = struct
  type 'a t = 'a Lwt.t

  let return = Lwt.return

  let map e f = Lwt.map f e

  let bind = Lwt.bind

  let both = Lwt.both
end
