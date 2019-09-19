(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type command =
  | CSchedule of RemoteScheduler.schedule_env
  | CWork of RemoteWorker.work_env

type command_keyword =
  | CKSchedule
  | CKWork
  | CKNone

let command_keyword_to_string (keyword : command_keyword) : string =
  match keyword with
  | CKSchedule -> "schedule"
  | CKWork -> "work"
  | CKNone -> ""

let string_to_command_keyword (str : string) : command_keyword =
  match str with
  | "schedule" -> CKSchedule
  | "work" -> CKWork
  | _ -> CKNone

let parse_command () =
  if Array.length Sys.argv < 2 then
    CKNone
  else
    string_to_command_keyword (String.lowercase_ascii Sys.argv.(1))

let parse_without_command options usage ~(keyword : command_keyword) =
  let args = ref [] in
  Arg.parse (Arg.align options) (fun x -> args := x :: !args) usage;
  match List.rev !args with
  | x :: rest when string_to_command_keyword x = keyword -> rest
  | args -> args

let parse_root (args : string list) : Path.t =
  match args with
  | [] -> Wwwroot.get None
  | [x] -> Wwwroot.get (Some x)
  | _ ->
    Printf.fprintf
      stderr
      "Error: please provide at most one root directory\n%!";
    exit 1

let parse_schedule_args () : command =
  let timeout = ref 9999 in
  let options =
    [("--timeout", Arg.Int (fun x -> timeout := x), " The timeout")]
  in
  let usage = "Usage: " ^ Sys.executable_name ^ " schedule <repo_root>" in
  let args = parse_without_command options usage ~keyword:CKSchedule in
  let (root : Path.t) = parse_root args in
  let bin_root = Path.make (Filename.dirname Sys.argv.(0)) in
  CSchedule
    RemoteScheduler.
      { (RemoteScheduler.default_env ~bin_root ~root) with timeout = !timeout }

let parse_work_args () : command =
  let key = ref "" in
  let timeout = ref 9999 in
  let options =
    [
      ("--key", Arg.String (fun x -> key := x), " The worker's key");
      ("--timeout", Arg.Int (fun x -> timeout := x), " The timeout");
    ]
  in
  let usage = "Usage: " ^ Sys.executable_name ^ " work <key>" in
  let args = parse_without_command options usage ~keyword:CKWork in
  let root = parse_root args in
  let bin_root = Path.make (Filename.dirname Sys.argv.(0)) in
  let check_id = Random_id.short_string () in
  CWork
    RemoteWorker.
      {
        bin_root;
        check_id;
        key = !key;
        root;
        timeout = !timeout;
        type_check = None;
      }

let parse_args () =
  match parse_command () with
  | CKNone
  | CKSchedule ->
    parse_schedule_args ()
  | CKWork -> parse_work_args ()

let () =
  let () = Daemon.check_entry_point () in
  let command = parse_args () in
  let _errors =
    match command with
    | CSchedule schedule_env ->
      (* TODO: RemoteScheduler.go schedule_env *)
      ignore schedule_env
    | CWork work_env ->
      (* TODO: RemoteWorker.go work_env *)
      ignore work_env
  in
  Exit_status.exit Exit_status.No_error
