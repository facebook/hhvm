(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** These are human-readable messages, shown at command-line and within the editor. *)
type t = {
  pid: int;  (** pid of the process that wrote this status *)
  message: string;  (** e.g. "typechecking 5/15 files" *)
  timestamp: float;
}

(** The caller must set this before attempting to send progress, otherwise exception *)
let root : Path.t option ref = ref None

let set_root (r : Path.t) : unit = root := Some r

let disable () : unit = root := Some Path.dummy_path

let server_progress_file () =
  match !root with
  | None -> failwith "ServerProgress.set_root must be called first"
  | Some root when Path.equal root Path.dummy_path -> None
  | Some root -> Some (ServerFiles.server_progress_file root)

let try_delete () : unit =
  match server_progress_file () with
  | None -> ()
  | Some server_progress_file ->
    (try Unix.unlink server_progress_file with
    | _ -> ())

(** This writes to the specified progress file. It first acquires
an exclusive (writer) lock. (Locks on unix are advisory; we trust
read_progress_file below to also acquire a lock). It overwrites
whatever was there before. In case of failure, it logs but is
silent. That's on the principle that defects in
progress-reporting should never break hh_server. *)
let write_file (t : t) : unit =
  match server_progress_file () with
  | None -> ()
  | Some server_progress_file ->
    let open Hh_json in
    let content =
      JSON_Object
        [
          ("pid", int_ t.pid);
          ("progress", string_ t.message);
          ("timestamp", float_ t.timestamp);
        ]
      |> json_to_multiline
    in
    (try Sys_utils.protected_write_exn server_progress_file content with
    | exn ->
      let e = Exception.wrap exn in
      Hh_logger.log
        "SERVER_PROGRESS_EXCEPTION(write) %s\n%s"
        (Exception.get_ctor_string e)
        (Exception.get_backtrace_string e |> Exception.clean_stack);
      HackEventLogger.server_progress_write_exn ~server_progress_file e;
      ())

(** This reads the specified progress file, which is assumed to exist.
It first acquires a non-exclusive (reader) lock. (Locks on unix are
advisory; we trust write_progress_file above to also acquire a writer
lock).  If there are failures, we log, and return a human-readable
string that indicates why. *)
let read () : t =
  let unknown =
    {
      pid = 0;
      message = "unknown hh_server state";
      timestamp = Unix.gettimeofday ();
    }
  in
  match server_progress_file () with
  | None -> failwith "ServerProgress.disable: can't read it"
  | Some server_progress_file ->
    let content = ref "[not yet read content]" in
    (try
       content := Sys_utils.protected_read_exn server_progress_file;
       let json = Some (Hh_json.json_of_string !content) in
       let pid = Hh_json_helpers.Jget.int_exn json "pid" in
       let message = Hh_json_helpers.Jget.string_exn json "progress" in
       let timestamp = Hh_json_helpers.Jget.float_exn json "timestamp" in
       (* If the status had been left behind on disk by a process that terminated without deleting it,
          well, we'll return the same 'unknown' as if the file didn't exist. *)
       let still_alive =
         try
           Unix.kill pid 0;
           true
         with
         | _ -> false
       in
       if still_alive then
         { pid; message; timestamp }
       else
         unknown
     with
    | exn ->
      let e = Exception.wrap exn in
      Hh_logger.log
        "SERVER_PROGRESS_EXCEPTION(read) %s\n%s\n%s"
        (Exception.get_ctor_string e)
        (Exception.get_backtrace_string e |> Exception.clean_stack)
        !content;
      HackEventLogger.server_progress_read_exn ~server_progress_file e;
      unknown)

let write ?(include_in_logs = true) fmt =
  let f message =
    begin
      if include_in_logs then Hh_logger.log "%s" message;
      let timestamp = Unix.gettimeofday () in
      write_file { pid = Unix.getpid (); message; timestamp }
    end
  in
  Printf.ksprintf f fmt

(* The message will look roughly like this:
   <operation> <done_count>/<total_count> <unit> <percent done> <extra>*)
let make_percentage_progress_message
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : string =
  let unit =
    if String.equal unit "" then
      unit
    else
      unit ^ " "
  in
  let percent = 100.0 *. float_of_int done_count /. float_of_int total_count in
  let main_message =
    Printf.sprintf
      "%s %d/%d %s(%.1f%%)"
      operation
      done_count
      total_count
      unit
      percent
  in
  match extra with
  | Some extra -> main_message ^ " " ^ extra
  | None -> main_message

let write_percentage
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : unit =
  write
    ~include_in_logs:false
    "%s"
    (make_percentage_progress_message
       ~operation
       ~done_count
       ~total_count
       ~unit
       ~extra)
