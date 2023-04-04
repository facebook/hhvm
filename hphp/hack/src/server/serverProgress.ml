(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type disposition =
  | DStopped [@value 1]
  | DWorking [@value 2]
  | DReady [@value 3]
[@@deriving show { with_path = false }, enum]

let _unused = (min_disposition, max_disposition)
(* to suppress "unused" warning *)

type t = {
  pid: int;
  disposition: disposition;
  message: string;
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
[read] below to also acquire a lock). It overwrites
whatever was there before. In case of failure, it logs but is
silent. That's on the principle that defects in
progress-reporting should never break hh_server. *)
let write_file (t : t) : unit =
  match server_progress_file () with
  | None -> ()
  | Some server_progress_file ->
    let open Hh_json in
    let { pid; disposition; message; timestamp } = t in
    let content =
      JSON_Object
        [
          ("pid", int_ pid);
          ("disposition", int_ (disposition_to_enum disposition));
          ("progress", string_ message);
          ("timestamp", float_ timestamp);
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
advisory; we trust [write_file] above to also acquire a writer
lock).  If there are failures, we log, and return a human-readable
string that indicates why. *)
let read () : t =
  let unknown =
    {
      pid = 0;
      disposition = DStopped;
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
       let disposition =
         Hh_json_helpers.Jget.int_opt json "disposition"
         |> Option.bind ~f:disposition_of_enum
         |> Option.value ~default:DReady
       in
       (* If the status had been left behind on disk by a process that terminated without deleting it,
          well, we'll return the same 'unknown' as if the file didn't exist. *)
       if Proc.is_alive ~pid ~expected:"" then
         { pid; message; disposition; timestamp }
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

let write ?(include_in_logs = true) ?(disposition = DWorking) fmt =
  let f message =
    begin
      if include_in_logs then Hh_logger.log "[progress] %s" message;
      let timestamp = Unix.gettimeofday () in
      write_file { pid = Unix.getpid (); disposition; message; timestamp }
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
  let percent =
    Float.round_down
      (1000.0 *. float_of_int done_count /. float_of_int total_count)
    /. 10.0 (* so that 999999/1000000 will show as 99.9%, not 100.0% *)
  in
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

type errors_file_error =
  | NothingYet
  | Complete of Telemetry.t [@printer (fun fmt _t -> fprintf fmt "Complete")]
  | Restarted
  | Stopped
  | Killed
  | Build_id_mismatch
[@@deriving show { with_path = false }]

let is_production_enabled = ref true

let enable_error_production (b : bool) : unit = is_production_enabled := b

let errors_file () =
  match !root with
  | None -> failwith "ServerProgress.set_root must be called first"
  | Some _ when not !is_production_enabled -> None
  | Some root when Path.equal root Path.dummy_path -> None
  | Some root -> Some (ServerFiles.errors_file root)

(** This is an internal module concerned with the binary format of the errors-file. *)
module ErrorsFile = struct
  (** The errors-file is a binary format consisting of a sequence of messages
  written by the hh_server process.

  The first two messages are [VersionHeader] followed by [Header], and these
  always exist (they are placed atomically at error-file creation by
  [ErrorsWrite.new_empty_file]). Then there are zero or more [Report], each
  one appended by a call to [ErrorsWrite.report]. Finally, there may be an [End],
  appended by a call to [ErrorsWrite.complete] or [unlink_after_stopped] or [new_empty_file].

  Each message consists of a 5-byte preamble (one sync byte, 4 size bites)
  followed by a marshalled [message] structure. *)
  type message =
    | VersionHeader of {
        version: string;
            (** from Build_id.build_revision, or empty if dev build or --ignore-hh-version *)
        extra: Hh_json.json;
            (** CARE! The hh_client binary might read the [VersionHeader] message that was written by either
            an older or a newer version of the hh_server binary. Therefore, it is impossible to
            change the datatype! Any new fields will have to be added to 'extra' if they're needed
            cross-version, or more commonly just placed in [Header] since it is version-safe. *)
      }
    | Header of {
        pid: int;
            (** the pid of the hh_server that wrote this file; clients check "sigkill 0 <pid>" to see if it's still alive *)
        cmdline: string;
            (** the /proc/pid/cmdline of hh_server; clients check that the current /proc/pid/cmdline is the same (proving that the pid hasn't been recycled) *)
        timestamp: float;  (** the time at which the typecheck began *)
        clock: Watchman.clock option;
            (** the watchclock at which the typecheck began, i.e. reflecting all file-changes up to here *)
      }
    | Report of {
        errors: Errors.error list Relative_path.Map.t;
        timestamp: float;
            (** the errors were detected by reading the files not later than this time. *)
      }
    | End of {
        error: errors_file_error;
        timestamp: float;
        log_message: string;
      }

  (** helper function to set current file position to 0, then do "f",
     then restore it to what it was before *)
  let with_pos0 (fd : Unix.file_descr) ~(f : unit -> 'a) =
    let pos = Unix.lseek fd 0 Unix.SEEK_CUR in
    let _ = Unix.lseek fd 0 Unix.SEEK_SET in
    let result = f () in
    let _ = Unix.lseek fd pos Unix.SEEK_SET in
    result

  (** helper function to acquire a lock on the whole file, then do "f",
   then release the lock *)
  let with_lock
      (fd : Unix.file_descr)
      (lock_command : Unix.lock_command)
      ~(f : unit -> 'a) =
    Utils.try_finally
      ~f:(fun () ->
        (* lockf is applied starting at the file-descriptors current position.
           We use "with_pos0" so that when we acquire or release the lock,
           we're locking from the start of the file through to (len=0) the end. *)
        with_pos0 fd ~f:(fun () ->
            Sys_utils.restart_on_EINTR (Unix.lockf fd lock_command) 0);
        f ())
      ~finally:(fun () ->
        with_pos0 fd ~f:(fun () ->
            Sys_utils.restart_on_EINTR (Unix.lockf fd Unix.F_ULOCK) 0);
        ())

  (** This helper acquires an exclusive lock on the file, appends the message, then releases the lock.
     It does not do any state validation - that's left to its caller. *)
  let write_message (fd : Unix.file_descr) (message : message) : unit =
    let payload = Marshal.to_bytes message [] in
    let preamble = Marshal_tools.make_preamble (Bytes.length payload) in
    with_lock fd Unix.F_LOCK ~f:(fun () ->
        Sys_utils.write_non_intr fd preamble 0 (Bytes.length preamble);
        Sys_utils.write_non_intr fd payload 0 (Bytes.length payload))

  let read_message (fd : Unix.file_descr) : message =
    let synthesize_end (error : errors_file_error) (log_message : string) :
        message =
      End { error; timestamp = Unix.gettimeofday (); log_message }
    in
    with_lock fd Unix.F_RLOCK ~f:(fun () ->
        let preamble =
          Sys_utils.read_non_intr fd Marshal_tools.expected_preamble_size
        in
        match preamble with
        | None -> synthesize_end NothingYet "no additional bytes"
        | Some preamble ->
          let size = Marshal_tools.parse_preamble preamble in
          (* This assert is in case the file is garbled, and we read a crazy-big size,
             to avoid allocating say a 20gb bytes array and having the machine get stuck. *)
          assert (size < 20_000_000);
          (match Sys_utils.read_non_intr fd size with
          | None -> synthesize_end Killed "no payload"
          | Some payload ->
            let message : message = Marshal.from_bytes payload 0 in
            message))
end

module ErrorsWrite = struct
  (** This structure represent's hh_server's current knowledge about the errors-file.
  Only hh_server should be manipulating the errors-file; hence, this knowledge is authoritative. *)
  type write_state =
    | Absent
        (** The errors-file either doesn't exist because this hh_server instance hasn't
        yet called [new_empty_file], or because it called [unlink_at_server_stop]. *)
    | Reporting of Unix.file_descr * int
        [@printer (fun fmt (_fd, n) -> fprintf fmt "Reporting[%d]" n)]
        (** The errors-file does exist,
        due to a previous call to [new_empty_file], and it's fine to call [report] or [complete]. *)
    | Closed
        (** The errors-file has an end-marker due to a call to [complete]. *)
  [@@deriving show { with_path = false }]

  (** This mutable value tracks the current state of the errors-file belonging to this
  instance of hh_server.
  The errors-file is deemed absent when hh_server starts up. Even if there had been a leftover
  errors-file from a previous hh_server instance, any attempt to read it will fail because it
  necessarily must have come from a now-dead PID. *)
  let write_state : write_state ref = ref Absent

  (** We print fds of error-files to the server log, showing their inode identity. *)
  let show_fd (fd : Unix.file_descr) : string =
    let stat = Unix.fstat fd in
    Printf.sprintf "fd%d:%d" stat.Unix.st_dev stat.Unix.st_ino

  (** This helper is called by [new_empty_file] and [unlink_at_server_stop]...
     1. It unlinks the current file, if any (in states [Reporting], [Closed])
     2. It calls the [after_unlink] callback
     3. It writes a End message if needed (in state [Reporting])
     4. It closes the file-descriptor for the current file (in state [Reporting]).

     For example: in the case of caller [new_empty_file], its [after_unlink] callback creates a new errors file.
     In this way, if a client with an existing file-descriptor should read a sentinel, then it knows
     for sure it can immediately close that file-descriptor and re-open the new errors file. *)
  let unlink_sentinel_close
      (error : errors_file_error)
      ~(log_message : string)
      ~(errors_file : string)
      ~(after_unlink : unit -> 'a) =
    begin
      try Unix.unlink errors_file with
      | _ -> ()
    end;
    let result = after_unlink () in
    begin
      match !write_state with
      | Reporting (fd, count) ->
        Hh_logger.log
          "Errors-file: ending old %s with its %d reports"
          (show_fd fd)
          count;
        ErrorsFile.write_message
          fd
          (ErrorsFile.End
             { error; timestamp = Unix.gettimeofday (); log_message });
        Unix.close fd
      | _ -> ()
    end;
    result

  let new_empty_file
      ~(clock : Watchman.clock option) ~(ignore_hh_version : bool) : unit =
    match errors_file () with
    | None -> ()
    | Some errors_file -> begin
      (* (1) unlink the old errors file, (2) atomically create a new errors-file with
         Version_header+Header messages in it, (3) write a End marker into the old errors file.

         **Atomicity** is so that a client can be assured that if they open an errors-file
         then it will necessarily have two errors in it.
         **Sentinel-after-new** is so that a client can be assured that if they encounter
         a sentinel then there's already a new errors-file ready to be read immediately
         or, if not, then the server must have died.

         Both mechanisms are there to make the client-side code easier to write! *)
      let pid = Unix.getpid () in
      let version =
        if ignore_hh_version then
          ""
        else
          Build_id.build_revision
      in
      let version_header =
        ErrorsFile.VersionHeader { version; extra = Hh_json.JSON_Object [] }
      in
      let header =
        ErrorsFile.Header
          {
            pid;
            cmdline = Proc.get_cmdline pid |> Result.ok_or_failwith;
            timestamp = Unix.gettimeofday ();
            clock;
          }
      in
      let fd =
        unlink_sentinel_close
          Restarted
          ~log_message:"new_empty_file"
          ~errors_file
          ~after_unlink:(fun () ->
            let fd =
              Sys_utils.atomically_create_and_init_file
                errors_file
                ~rd:false
                ~wr:true
                0o666
                ~init:(fun fd ->
                  ErrorsFile.write_message fd version_header;
                  ErrorsFile.write_message fd header)
            in
            match fd with
            | None ->
              failwith "Errors-file was created by someone else under our feet"
            | Some fd ->
              Hh_logger.log "Errors-file: starting new %s" (show_fd fd);
              fd)
      in
      write_state := Reporting (fd, 0)
    end

  let report (errors : Errors.t) : unit =
    match errors_file () with
    | None -> ()
    | Some _ -> begin
      match !write_state with
      | Absent
      | Closed ->
        failwith ("Cannot report in state " ^ show_write_state !write_state)
      | Reporting _ when Errors.is_empty ~drop_fixmed:true errors -> ()
      | Reporting (fd, n) ->
        let n = n + 1 in
        if n <= 5 then
          Hh_logger.log
            "Errors-file: report#%d on %s: %d new errors%s"
            n
            (show_fd fd)
            (Errors.count errors)
            (if n = 5 then
              " [won't report any more this typecheck...]"
            else
              "");
        (* sort and dedupe *)
        let errors =
          errors
          |> Errors.drop_fixmed_errors_in_files
          |> Errors.as_map
          |> Relative_path.Map.map ~f:Errors.sort
        in
        ErrorsFile.write_message
          fd
          (ErrorsFile.Report { timestamp = Unix.gettimeofday (); errors });
        write_state := Reporting (fd, n)
    end

  let complete (telemetry : Telemetry.t) : unit =
    match errors_file () with
    | None -> ()
    | Some _ -> begin
      match !write_state with
      | Absent
      | Closed ->
        failwith ("Cannot complete in state " ^ show_write_state !write_state)
      | Reporting (fd, n) ->
        Hh_logger.log
          "Errors-file: completing %s after %d reports"
          (show_fd fd)
          n;
        ErrorsFile.write_message
          fd
          (ErrorsFile.End
             {
               error = Complete telemetry;
               timestamp = Unix.gettimeofday ();
               log_message = "complete";
             });
        write_state := Closed
    end

  let unlink_at_server_stop () : unit =
    match errors_file () with
    | None -> ()
    | Some errors_file ->
      unlink_sentinel_close
        Stopped
        ~log_message:"unlink"
        ~errors_file
        ~after_unlink:(fun () -> ());
      write_state := Absent

  let get_state_FOR_TEST () : string = show_write_state !write_state

  let create_file_FOR_TEST ~(pid : int) ~(cmdline : string) : unit =
    let fd =
      Unix.openfile
        (Option.value_exn (errors_file ()))
        [Unix.O_WRONLY; Unix.O_CREAT; Unix.O_TRUNC]
        0o666
    in
    ErrorsFile.write_message
      fd
      (ErrorsFile.VersionHeader { version = ""; extra = Hh_json.JSON_Object [] });
    ErrorsFile.write_message
      fd
      (ErrorsFile.Header { pid; cmdline; timestamp = 0.0; clock = None });
    Unix.close fd
end

module ErrorsRead = struct
  type log_message = string

  type open_success = {
    pid: int;
    timestamp: float;
    clock: Watchman.clock option;
  }

  type read_result =
    ( Errors.error list Relative_path.Map.t * float,
      errors_file_error * log_message )
    result

  let openfile (fd : Unix.file_descr) :
      (open_success, errors_file_error * log_message) result =
    let message1 = ErrorsFile.read_message fd in
    let message2 = ErrorsFile.read_message fd in
    match (message1, message2) with
    | ( ErrorsFile.VersionHeader { version; _ },
        ErrorsFile.Header { pid; cmdline; clock; timestamp } ) ->
      if
        String.length version > 16
        && String.length Build_id.build_revision > 16
        && not (String.equal version Build_id.build_revision)
      then
        (* This is a version mismatch which we can't ignore. (We always ignore mismatch from dev-builds...
           version="" means a buck dev build, and version.length<=16 means a dune dev build.) *)
        let msg =
          Printf.sprintf
            "errors-file is version %s, but we are %s"
            version
            Build_id.build_revision
        in
        Error (Build_id_mismatch, msg)
      else if not (Proc.is_alive ~pid ~expected:cmdline) then
        Error (Killed, "Errors-file is from defunct PID")
      else
        Ok { pid; clock; timestamp }
    | _ -> failwith "impossible message combination"

  let read_next_errors (fd : Unix.file_descr) : read_result =
    match ErrorsFile.read_message fd with
    | ErrorsFile.VersionHeader _
    | ErrorsFile.Header _ ->
      failwith
        "do ServerProgress.ErrorsRead.openfile before read_next_error or ServerProgressLwt.watch_errors_file"
    | ErrorsFile.Report { errors; timestamp } -> Ok (errors, timestamp)
    | ErrorsFile.End { error; log_message; _ } -> Error (error, log_message)
end

let validate_errors_DELETE_THIS_SOON ~(expected : Errors.t) : string =
  (* helper to combine batches of errors together *)
  let rec reader fd acc =
    match ErrorsRead.read_next_errors fd with
    | Ok (errors, _timestamp) ->
      let errors = Relative_path.Map.values errors |> List.concat in
      reader fd (errors @ acc)
    | Error _ -> acc
  in

  (* helper to diff two sorted lists: "list_diff ([],[]) xs ys" will return a pair
     (only_xs, only_ys) with those elements that are only in xs, and those only in ys. *)
  let rec list_diff (only_xs, only_ys) xs ys ~compare =
    match (xs, ys) with
    | ([], ys) -> (only_xs, ys @ only_ys)
    | (xs, []) -> (xs @ only_xs, only_ys)
    | (x :: xs, y :: ys) ->
      let c = compare x y in
      if c < 0 then
        list_diff (x :: only_xs, ys) xs (y :: ys) ~compare
      else if c > 0 then
        list_diff (only_xs, y :: only_ys) (x :: xs) ys ~compare
      else
        list_diff (only_xs, only_ys) xs ys ~compare
  in

  (* helper to log up to first five errors, and return their codes *)
  let code_and_log_first_five errors msg =
    let len = List.length errors in
    let errors = List.take errors 5 in
    let codes = List.map errors ~f:User_error.get_code in
    if len > 0 then
      Hh_logger.log
        "%s (%d errors)\n%s"
        msg
        len
        (errors
        |> List.map ~f:(fun { User_error.claim = (pos, msg); code; _ } ->
               let suffix = Pos.filename pos |> Relative_path.suffix in
               Printf.sprintf "  err [%d] %s %s" code suffix msg)
        |> String.concat ~sep:"\n");
    (len, codes)
  in

  match errors_file () with
  | None -> "disabled"
  | Some errors_file ->
    (* The expected one needs to have fixmes droped, and duplicates dropped, and be sorted *)
    let expected = Errors.get_sorted_error_list expected in
    (* Our errors-file has already dropped fixmes, sorted-within-file, and dropped duplicates.
       We'll do Errors.sort on it now to do a global sort by filename, to match "expected". *)
    let fd = Unix.openfile errors_file [Unix.O_RDONLY; Unix.O_CREAT] 0o666 in
    let fd_str = ErrorsWrite.show_fd fd in
    let _ = ErrorsRead.openfile fd in
    let actual = reader fd [] |> Errors.sort in
    Unix.close fd;
    (* They should be identical. Let's break down which errors are absent from the errors-file
       which should be there, and which are extra and shouldn't be there. *)
    let (absent, extra) =
      list_diff ([], []) expected actual ~compare:Errors.compare
    in
    let (num_absent, codes_absent) =
      code_and_log_first_five absent "Absent from errors_file"
    in
    let (num_extra, codes_extra) =
      code_and_log_first_five extra "Extraneous in errors_file"
    in
    if num_absent = 0 && num_extra = 0 then begin
      Hh_logger.log
        "Errors-file: Good news everyone! file %s is correct for this typecheck"
        fd_str;
      "good"
    end else begin
      let telemetry =
        Telemetry.create ()
        |> Telemetry.int_ ~key:"absent_count" ~value:num_absent
        |> Telemetry.int_list ~key:"absent_codes" ~value:codes_absent
        |> Telemetry.int_ ~key:"extra_count" ~value:num_extra
        |> Telemetry.int_list ~key:"extra_codes" ~value:codes_extra
      in
      HackEventLogger.errors_file_mismatch (List.length expected) telemetry;
      if num_absent = 0 then
        "extra items"
      else if num_extra = 0 then
        "absent items"
      else
        "wrong items"
    end
