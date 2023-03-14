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
  server_progress: string;  (** e.g. "typechecking 5/15 files" *)
  server_warning: string option;  (** e.g. "typechecking will be slow" *)
  server_timestamp: float;
}

(** The caller must set this before attempting to send progress, otherwise exception *)
let root : Path.t option ref = ref None

(** latest_progress is the progress message we most recently wrote to server_progress_file *)
let latest_progress : string ref = ref "server about to start up"

(** latest_warning is the warning message we most recently wrote to server_progress_file *)
let latest_warning : string option ref = ref None

let set_root (r : Path.t) : unit = root := Some r

let disable () : unit = root := Some Path.dummy_path

let server_progress_file () =
  match !root with
  | None -> failwith "ServerProgress.set_root must be called first"
  | Some root when Path.equal root Path.dummy_path -> None
  | Some root -> Some (ServerFiles.server_progress_file root)

(** This writes to the specified progress file. It first acquires
an exclusive (writer) lock. (Locks on unix are advisory; we trust
read_progress_file below to also acquire a lock). It overwrites
whatever was there before. In case of failure, it logs but is
silent. That's on the principle that defects in
progress-reporting should never break hh_server. *)
let write (t : t) : unit =
  match server_progress_file () with
  | None -> ()
  | Some server_progress_file ->
    let open Hh_json in
    let content =
      JSON_Object
        [
          ( "warning",
            Option.value_map t.server_warning ~default:JSON_Null ~f:string_ );
          ("progress", string_ t.server_progress);
          ("timestamp", float_ t.server_timestamp);
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
  match server_progress_file () with
  | None -> failwith "ServerProgress.disable: can't read it"
  | Some server_progress_file ->
    let content = ref "[not yet read content]" in
    (try
       content := Sys_utils.protected_read_exn server_progress_file;
       let json = Some (Hh_json.json_of_string !content) in
       let server_progress = Hh_json_helpers.Jget.string_exn json "progress" in
       let server_warning = Hh_json_helpers.Jget.string_opt json "warning" in
       let server_timestamp = Hh_json_helpers.Jget.float_exn json "timestamp" in
       { server_progress; server_warning; server_timestamp }
     with
    | exn ->
      let e = Exception.wrap exn in
      Hh_logger.log
        "SERVER_PROGRESS_EXCEPTION(read) %s\n%s\n%s"
        (Exception.get_ctor_string e)
        (Exception.get_backtrace_string e |> Exception.clean_stack)
        !content;
      HackEventLogger.server_progress_read_exn ~server_progress_file e;
      {
        server_progress = "unknown hh_server state";
        server_warning = None;
        server_timestamp = Unix.gettimeofday ();
      })

let write_latest () : unit =
  write
    {
      server_progress = !latest_progress;
      server_warning = !latest_warning;
      server_timestamp = Unix.gettimeofday ();
    }

let send_warning s =
  begin
    match (!latest_warning, s) with
    | (Some latest, Some s) when String.equal latest s -> ()
    | (None, None) -> ()
    | (_, _) ->
      latest_warning := s;
      write_latest ()
  end;
  ()

let send_progress ?(include_in_logs = true) fmt =
  let f s =
    if include_in_logs then Hh_logger.log "%s" s;
    if not (String.equal !latest_progress s) then begin
      latest_progress := s;
      write_latest ()
    end;
    ()
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

let send_percentage_progress
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : unit =
  send_progress
    ~include_in_logs:false
    "%s"
    (make_percentage_progress_message
       ~operation
       ~done_count
       ~total_count
       ~unit
       ~extra)
