(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type proc_stat = {
  cmdline: string;
  ppid: int;
}

let cmdline_delimiter_re = Str.regexp "\x00"

(* Takes a PID and returns the full command line the process was started with *)
let get_cmdline (pid : int) : (string, string) result =
  (* NOTE: Linux's OS type is Unix *)
  if not Sys.unix then
    Error "Getting cmdline is not implemented for non-Unix OS types"
  else
    let cmdline_path = Printf.sprintf "/proc/%d/cmdline" pid in
    try
      let line =
        Str.global_replace cmdline_delimiter_re " " (Disk.cat cmdline_path)
      in
      Ok line
    with
    | e ->
      let error =
        Printf.sprintf
          "No 'cmdline' file found for PID %d: '%s'"
          pid
          (Exn.to_string e)
      in
      Error error

(* Takes a PID and returns the information about the process, including
   the name and the PID of the parent process (PPID) *)
let get_proc_stat (pid : int) : (proc_stat, string) result =
  (* NOTE: Linux's OS type is Unix *)
  if not Sys.unix then
    Error "Getting cmdline is not implemented for non-Unix OS types"
  else
    let stat_path = Printf.sprintf "/proc/%d/stat" pid in
    try
      let stat = Scanf.Scanning.from_string (Disk.cat stat_path) in
      try
        let record =
          Scanf.bscanf
            stat
            "%d (%s@) %c %d"
            (fun _my_pid _comm _state ppid : (proc_stat, string) result ->
              match get_cmdline pid with
              | Ok cmdline -> Ok { cmdline; ppid }
              | Error err -> Error err)
        in
        record
      with
      | e ->
        let error =
          Printf.sprintf
            "Error reading 'stat' for PID %d: %s"
            pid
            (Exn.to_string e)
        in
        Error error
    with
    | e ->
      let error =
        Printf.sprintf
          "No 'stat' file found for PID %d: '%s'"
          pid
          (Exn.to_string e)
      in
      Error error

let get_proc_stack
    ?(max_depth : int = -1) ?(max_length : int = Int.max_value) (pid : int) :
    (string list, string) result =
  let prepare_cmdline (cmdline : string) : string =
    let cmdline = Caml.String.trim cmdline in
    if max_length >= String.length cmdline then
      cmdline
    else
      Caml.String.trim (String.sub cmdline ~pos:0 ~len:max_length) ^ "..."
  in
  (* We could have max_depth as optional, but then everybody would have to pass in None *)
  (* let max_depth = match max_depth with | None -> -1 | Some max_depth -> max_depth in *)
  let rec build_proc_stack
      (curr_pid : int) (proc_stack : string list) (counter : int) :
      (string list, string) result =
    if curr_pid = 0 then
      Ok proc_stack
    else if counter = max_depth then
      Ok proc_stack
    else
      match get_proc_stat curr_pid with
      | Ok stat ->
        build_proc_stack
          stat.ppid
          (prepare_cmdline stat.cmdline :: proc_stack)
          (counter + 1)
      | Error e -> Error e
  in
  build_proc_stack pid [] 0

(** There's no reliable way to tell whether an arbitrary PID is still alive.
That's because after the process has long since died, its PID might be recycled
for use by another process. This routine makes a best-effort attempt:
1. Can we read /proc/<pid>/cmdline? - yes if it's alive or still being held onto.
2. Is "expected" a substring of cmdline? - this is our best-effort protection
against pid recyling, since the chance that it gets recycled *and* the recycled one
has the process-name we were expecting just seems pretty low. Note that cmdline
will be empty if the process has been zombified. Also that if a process modifies
its argv then the cmdline will reflect that. In effect, cmdline is the command-line
that the process wants you to see.
3. Can we send SIGKILL 0 to it? If this succeeds without throwing an exception,
then the process is alive. *)
let is_alive ~(pid : int) ~(expected : string) : bool =
  if not Sys.unix then
    failwith "is_alive only implemented on Linux"
  else begin
    (* 1. Can we fetch /proc/<pid>/cmdline ? *)
    match get_cmdline pid with
    | Error _ -> false
    | Ok cmdline ->
      (* 2. Is /proc/<pid>/cmdline the process-name that we expected? *)
      (* To consider: it would be more accurate, but less usable, to test whether /proc/stat
         records the same starttime as expected. See 'man proc' for more details. *)
      if String.is_substring cmdline ~substring:expected then
        try
          (* 3. Does "SIGKILL <pid> 0" succeed to send a message to the process? *)
          Unix.kill pid 0;
          true
        with
        | _ -> false
      else
        false
  end
