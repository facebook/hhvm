(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Future

type error_mode =
  | Process_failure of {
      status: Unix.process_status;
      stderr: string;
    }
  | Timed_out of {
      stdout: string;
      stderr: string;
    }
  | Process_aborted
  | Transformer_raised of Exception.t

type process_error = Process_types.invocation_info * error_mode

module ProcessError = struct
  type config = process_error

  type t = process_error

  let create config = config

  let to_string ((info, error_mode) : t) : string =
    let info =
      Printf.sprintf
        "(%s [%s])"
        info.Process_types.name
        (String.concat ~sep:", " info.Process_types.args)
    in
    let status_string s =
      match s with
      | Unix.WEXITED i -> Printf.sprintf "(%s WEXITED %d)" info i
      | Unix.WSIGNALED i -> Printf.sprintf "(%s WSIGNALED %d)" info i
      | Unix.WSTOPPED i -> Printf.sprintf "(%s WSTOPPED %d)" info i
    in
    match error_mode with
    | Process_failure { status; stderr } ->
      Printf.sprintf
        "Process_failure(%s, stderr: %s)"
        (status_string status)
        stderr
    | Timed_out { stdout; stderr } ->
      Printf.sprintf
        "Timed_out(%s (stdout: %s) (stderr: %s))"
        info
        stdout
        stderr
    | Process_aborted -> Printf.sprintf "Process_aborted(%s)" info
    | Transformer_raised e ->
      Printf.sprintf
        "Transformer_raised(%s %s)"
        info
        (Exception.get_ctor_string e)

  let to_string_verbose ((info, error_mode) : t) : Future.verbose_error =
    let Process_types.{ name; args; env; stack = Utils.Callstack stack } =
      info
    in
    let env = Process.env_to_string env in
    let stack = stack |> Exception.clean_stack in
    let cmd_and_args =
      Printf.sprintf "`%s %s`" name (String.concat ~sep:" " args)
    in
    match error_mode with
    | Process_failure { status; stderr } ->
      let status =
        match status with
        | Unix.WEXITED i -> Printf.sprintf "exited with code %n" i
        | Unix.WSIGNALED i -> Printf.sprintf "killed with signal %n" i
        | Unix.WSTOPPED i -> Printf.sprintf "stopped with signal %n" i
      in
      {
        message = Printf.sprintf "%s - %s\n%s\n" cmd_and_args status stderr;
        environment = Some env;
        stack = Utils.Callstack stack;
      }
    | Timed_out { stdout; stderr } ->
      {
        message =
          Printf.sprintf
            "%s timed out\nSTDOUT:\n%s\nSTDERR:\n%s\n"
            cmd_and_args
            stdout
            stderr;
        environment = Some env;
        stack = Utils.Callstack stack;
      }
    | Process_aborted ->
      {
        message = Printf.sprintf "%s aborted" cmd_and_args;
        environment = None;
        stack = Utils.Callstack stack;
      }
    | Transformer_raised e ->
      let stack =
        (Exception.get_backtrace_string e |> Exception.clean_stack)
        ^ "-----\n"
        ^ stack
      in
      {
        message =
          Printf.sprintf
            "%s - unable to process output - %s"
            cmd_and_args
            (Exception.get_ctor_string e);
        environment = None;
        stack = Utils.Callstack stack;
      }
end

let make
    ?(timeout : int option)
    (process : Process_types.t)
    (transformer : string -> 'value) : 'value Future.t =
  let get_value ~(timeout : int) :
      ('value, (module Future.Error_instance)) result =
    let info = process.Process_types.info in
    let res =
      match Process.read_and_wait_pid ~timeout process with
      | Ok { Process_types.stdout; _ } ->
        begin
          try Ok (transformer stdout)
          with e ->
            let e = Exception.wrap e in
            Error (info, Transformer_raised e)
        end
      | Error (Process_types.Abnormal_exit { status; stderr; _ }) ->
        Error (info, Process_failure { status; stderr })
      | Error (Process_types.Timed_out { stdout; stderr }) ->
        Error (info, Timed_out { stdout; stderr })
      | Error Process_types.Overflow_stdin -> Error (info, Process_aborted)
    in
    Result.map_error res ~f:(fun (e : process_error) ->
        Future.create_error_instance (module ProcessError) e)
  in
  let is_value_ready () = Process.is_ready process in
  match timeout with
  | Some v -> Future.make ~timeout:v (get_value, is_value_ready)
  | None -> Future.make (get_value, is_value_ready)
