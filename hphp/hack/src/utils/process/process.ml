(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Ocaml_overrides
open Stack_utils

module Entry = struct

  type 'param t = ('param, unit, unit) Daemon.entry

  let register name entry =
    let daemon_entry =
      Daemon.register_entry_point name begin fun params _channels ->
        entry params
    end in
    daemon_entry

end

let chunk_size = 65536

(** In the blocking read_and_wait_pid call, we alternate between
 * non-blocking consuming of output and a nonblocking waitpid.
 * To avoid pegging the CPU at 100%, sleep for a short time between
 * those. *)
let sleep_seconds_per_retry = 0.04

(** Reuse the buffer for reading. Just an allocation optimization. *)
let buffer = Bytes.create chunk_size

let make_result status stdout stderr =
  let open Process_types in
  match status with
  | Unix.WEXITED 0 ->
    Ok (stdout, stderr)
  | Unix.WEXITED _
  | Unix.WSIGNALED _
  | Unix.WSTOPPED _ ->
    Error (Process_exited_abnormally (status, stdout, stderr))

(** Read from the FD if there is something to be read. FD is a reference
 * so when EOF is read from it, it is set to None. *)
let rec maybe_consume ?(max_time=0.0) fd_ref acc =
  if max_time < 0.0 then
    ()
  else
    let start_t = Unix.time () in
    Option.iter !fd_ref ~f:begin fun fd ->
      match Unix.select [fd] [] [] max_time with
      | [], _, _ -> ()
      | _ ->
        let bytes_read = Unix.read fd buffer 0 chunk_size in
        if bytes_read = 0 then begin
          (** EOF reached. *)
          Unix.close fd;
          fd_ref := None
        end else
          let chunk = String.sub buffer 0 bytes_read in
          Stack.push chunk acc;
          let consumed_t = Unix.time () -. start_t in
          let max_time = max_time -. consumed_t in
          maybe_consume ~max_time fd_ref acc
    end

let filter_none refs =
  List.fold_left refs ~init:[] ~f:(fun acc ref ->
    match !ref with
    | None -> acc
    | Some x -> x :: acc
  )

(** Non-blockingly consumes from pipes and non-blockingly waitpids.
 * Accumulators and process_status references are mutated accordingly. *)
let read_and_wait_pid_nonblocking process =
  let open Process_types in
  let {
  stdin_fd = _stdin_fd;
  stdout_fd;
  stderr_fd;
  process_status;
  acc;
  acc_err; _ } = process in
  match !process_status with
  | Process_aborted _
  | Process_exited _ ->
    ()
  | Process_running pid ->
    maybe_consume stdout_fd acc;
    maybe_consume stderr_fd acc_err;
    match Unix.waitpid [Unix.WNOHANG] pid with
    | 0, _ ->
      ()
    | _, status ->
      let () = process_status := Process_exited status in
      (** Process has exited. Non-blockingly consume residual output. *)
      let () = maybe_consume stdout_fd acc in
      let () = maybe_consume stderr_fd acc_err in
      ()

let is_ready process =
  read_and_wait_pid_nonblocking process;
  let open Process_types in
  match !(process.process_status) with
  | Process_running _ -> false
  | Process_aborted Input_too_large
  | Process_exited _ -> true

let kill_and_cleanup_fds pid fds =
  Unix.kill pid Sys.sigkill;
  let maybe_close fd_ref = Option.iter !fd_ref ~f:begin fun fd ->
    Unix.close fd;
    fd_ref := None
  end in
  List.iter fds ~f:maybe_close

(**
 * Consumes from stdout and stderr pipes and waitpids on the process.
 * Returns immediately if process has already been waited on (so this
 * function is idempotent).
 *
 * The implementation is a little complicated because:
 *   (1) The pipe can get filled up and the child process will pause
 *       until it's emptied out.
 *   (2) If the child process itself forks a grandchild, the
 *       granchild will unknowingly inherit the pipe's file descriptors;
 *       in this case, the pipe will not provide an EOF as you'd expect.
 *
 * Due to (1), we can't just blockingly waitpid followed by reading the
 * data from the pipe.
 *
 * Due to (2), we can't just read data from the pipes until an EOF is
 * reached and then do a waitpid.
 *
 * We must do some weird alternating between them.
 *)
let rec read_and_wait_pid ~retries process =
  let open Process_types in
  let {
  stdin_fd = _stdin_fd;
  stdout_fd;
  stderr_fd;
  process_status;
  acc;
  acc_err; _} = process in
  read_and_wait_pid_nonblocking process;
  match !process_status with
  | Process_exited status ->
    make_result status (Stack.merge_bytes acc) (Stack.merge_bytes acc_err)
  | Process_aborted Input_too_large ->
    Error Process_aborted_input_too_large
  | Process_running pid ->
  let fds = filter_none [stdout_fd; stderr_fd;] in
  if fds = []
  then
    (** EOF reached for all FDs. Blocking wait. *)
    let _, status = Unix.waitpid [] pid in
    let () = process_status := Process_exited status in
    make_result status (Stack.merge_bytes acc) (Stack.merge_bytes acc_err)
  else
    (** Consume output to clear the buffers which might
     * be blocking the process from continuing. *)
    let () = maybe_consume ~max_time:(sleep_seconds_per_retry /. 2.0) stdout_fd acc in
    let () = maybe_consume ~max_time:(sleep_seconds_per_retry /. 2.0) stderr_fd acc_err in
    (** EOF hasn't been reached for all FDs. Here's where we switch from
     * reading the pipes to attempting a non-blocking waitpid. *)
    match Unix.waitpid [Unix.WNOHANG] pid with
    | 0, _ ->
      if retries <= 0 then
        let () = kill_and_cleanup_fds pid [stdout_fd; stderr_fd] in
        Error (Timed_out
          ((Stack.merge_bytes acc), (Stack.merge_bytes acc_err)))
      else
        (** And here we switch from waitpid back to reading. *)
        read_and_wait_pid ~retries:(retries - 1) process
    | _, status ->
      (** Process has exited. Non-blockingly consume residual output. *)
      let () = maybe_consume stdout_fd acc in
      let () = maybe_consume stderr_fd acc_err in
      let () = process_status := Process_exited status in
      make_result status (Stack.merge_bytes acc) (Stack.merge_bytes acc_err)

let read_and_wait_pid ~timeout process =
  let retries = int_of_float @@
    (float_of_int timeout) /. sleep_seconds_per_retry in
  read_and_wait_pid ~retries process

let failure_msg failure =
  let open Process_types in
  match failure with
    | Timed_out (stdout, stderr) ->
      Printf.sprintf "Process timed out. stdout:\n%s\nstderr:\n%s\n"
      stdout stderr
    | Process_exited_abnormally (_, stdout, stderr) ->
      Printf.sprintf "Process exited abnormally. stdout:\n%s\nstderr:\n%s\n"
      stdout stderr
    | Process_aborted_input_too_large ->
      "Process_aborted_input_too_large"

let send_input_and_form_result
?input ~info ~pid ~stdin_parent ~stdout_parent ~stderr_parent =
  let open Process_types in
  let input_failed = match input with
    | None -> false
    | Some input ->
      let written = Unix.write stdin_parent input 0 (String.length input) in
      written <> String.length input
  in
  let process_status = if input_failed then begin
    Unix.kill pid Sys.sigkill;
    Process_aborted Input_too_large
  end
  else begin
    Process_running pid
  end
  in
  Unix.close stdin_parent;
  {
    info;
    stdin_fd = ref @@ None;
    stdout_fd = ref @@ Some stdout_parent;
    stderr_fd = ref @@ Some stderr_parent;
    acc = Stack.create ();
    acc_err = Stack.create ();
    process_status = ref @@ process_status;
  }


let exec_no_chdir prog ?input ?env args =
  let info = {
    Process_types.name = prog;
    args = args;
  } in
  let args = Array.of_list (prog :: args) in
  let stdin_child, stdin_parent = Unix.pipe () in
  let stdout_parent, stdout_child = Unix.pipe () in
  let stderr_parent, stderr_child = Unix.pipe () in
  Unix.set_close_on_exec stdin_parent;
  Unix.set_close_on_exec stdout_parent;
  Unix.set_close_on_exec stderr_parent;
  let pid = match env with
  | None ->
    Unix.create_process prog args stdin_child stdout_child stderr_child
  | Some env ->
    (* deduping the env is not necessary. glibc putenv/getenv will grab the first
     * one *)
    let fullenv = Array.append (Array.of_list env) (Unix.environment ()) in
    Unix.create_process_env prog args
      fullenv stdin_child stdout_child stderr_child
  in
  Unix.close stdin_child;
  Unix.close stdout_child;
  Unix.close stderr_child;
  send_input_and_form_result ?input ~info ~pid ~stdin_parent
    ~stdout_parent ~stderr_parent

let register_entry_point = Entry.register

let run_entry ?input entry params =
  let stdin_child, stdin_parent = Unix.pipe () in
  let stdout_parent, stdout_child = Unix.pipe () in
  let stderr_parent, stderr_child = Unix.pipe () in
  let info = {
    Process_types.name = Daemon.name_of_entry entry;
    args = [];
  } in
  let { Daemon.pid; _ } as daemon = Daemon.spawn
    (stdin_child, stdout_child, stderr_child) entry params in
  Daemon.close daemon;
  send_input_and_form_result ?input ~info ~pid ~stdin_parent
    ~stdout_parent ~stderr_parent

let chdir_main (cwd, prog, env_opt, args) =
  Unix.chdir cwd;
  let args = Array.of_list (prog :: args) in
  Unix.execvpe prog args (Array.of_list @@ Option.value env_opt ~default:[])

let chdir_entry = Entry.register "chdir_main" chdir_main


(** Exec the given program with these args. If input is set, send it to
 * the stdin on the spawned program and then close the file descriptor.
 *
 * Note: See Input_too_large
 *)
let exec_ ~cwd ~prog ~(input:string option) ~env ~args =
  match cwd with
  | None ->
    exec_no_chdir prog ?input ?env args
  | Some cwd ->
    run_entry ?input chdir_entry (cwd, prog, env, args)

let exec ?cwd prog ?input ?env args =
  exec_ ~cwd ~prog ~input ~env ~args

let exec_with_replacement_env ?cwd prog ~env ?input args =
  exec_ ~cwd ~prog ~input ~env:(Some env) ~args
