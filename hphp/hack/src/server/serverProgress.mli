(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

(** All functions in this file will throw unless you've already called set_root
at the start of your process. *)
val set_root : Path.t -> unit

(** use this in tests, instead of set_root,
to disable progress-logging and error-streaming. *)
val disable : unit -> unit

(** Progress is a file in /tmp/hh_server/<repo>.progress.json which is written
by monitor+server. It lives from the moment the monitor starts up until the
moment it finally dies or is killed. You should only read it by the `read`
call, since that protects against races. Anyone at any time can read this
file to learn the current state. The state is represented solely as a
human-readable string to be shown to the user in the CLI or VSCode status bar.
It specifically shouldn't be acted upon in code -- it's slightly handwavey
in places (e.g. there's an interval from when a server dies until the monitor
realizes that fact where attempting to read will say "unknown"). *)
type t = {
  pid: int;
  message: string;  (** e.g. "typechecking 5/15 files" *)
  timestamp: float;
}

val read : unit -> t

(* This is basically signature of "Printf.printf" *)
val write : ?include_in_logs:bool -> ('a, unit, string, unit) format4 -> 'a

val write_percentage :
  operation:string ->
  done_count:int ->
  total_count:int ->
  unit:string ->
  extra:string option ->
  unit

(** Call this upon monitor shutdown to delete the progress file *)
val try_delete : unit -> unit

(**********************************************************************
  OVERVIEW OF STREAMING ERRORS FILE

  Errors is a file at /tmp/hh_server/<repo>/errors.bin which is written
  by server during a typecheck. The file is first created when the server
  starts its first typecheck, gets errors appended to it as the typecheck
  discovers them, is unlinked and then another one with the same name
  created each time the server starts another typecheck, and the file is finally
  unlinked upon server exit. A "typecheck" can be thought of as a typecheck of the
  entire program, and hence the errors file will contain every error reported in WWW.
  But in reality the server does tricks to typecheck a smaller number of files just
  so long as it still manages to report every error into the file.

  Therefore: a client can at any time open an file descriptor to this file,
  and `tail -f -n +1` (i.e. read from the start and then follow) to follow
  the typecheck that is currently underway or most recently completed, from its
  start to its end. It will still be able to read the file descriptor through
  to its end even if, in the meantime, a new typecheck has started and the old
  file has been unlinked. Note that a given file-descriptor will only ever point
  to a file which monitonically grows.

  The server can have its typecheck interrupted. Some interruptions like watchman
  will cause the current typecheck to be cancelled, then a new typecheck started.
  In this case the existing errors file will be unlinked, a new errors file created
  for the new typecheck, and a sentinel will be written into the old file descriptor
  to show that it ended before completing. Other interruptions, e.g. from
  `hh --type-at-pos`, will have no effect: the current typecheck file can continue
  being read just fine. If a client reads a file descriptor on its sentinel, that is
  a guarantee that a new errors file is already in place and can be opened (or,
  if no errors file exists, that can only be because the server has terminated).

  If the file does not exist, the client knows that either there is no server, or there
  is one but it has not yet started typechecking (it might still be loading saved state).
  In either case it is appropriate for the client to clientConnect {autostart_server=true}
  in the normal way, until it establishes a connection to the server, and then it can
  wait for the errors file.


  SEMANTICS OF ERRORS-FILE CONTENTS

  * An errors-file is tied to a particular watchman clock value. It reflects
    all file-changes that happened prior to the clock. (As to whether it reflects
    any file-changes that happened after the clock, that is impossible to tell.)
  * As mentioned above, when the errors-file is complete, it contains the full
    set of errors that hh_server knows about for the project.
  * The error file is in a binary format consisting of a header followed by a series of
    Errors.error list Relative_path.map along with timestamp,
    followed by an "end sentinel" if the typecheck has finished or been interrupted.
    Let's call each of these maps an error report. Each encompasses
    several files, and for each file it has a list of errors. Currently we make
    one report for all "duplicate name" errors across the project if any, followed by
    one report per batch that had errors. This means that a file might be mentioned
    twice, once in a "duplicate name" report, once later. This will change in future
    so that each file is reported only once.
  * It currently does not write empty reports, though that might change in future
    (e.g. we might decide to write "no errors in file a.php" for a file which
    previously did have errors).
  * Within a single report, the error list has been sorted and de-duped.
*)

(** If we don't succeed in reading the next errors report, here's why. *)
type errors_file_error =
  | NothingYet
      (** There are no new errors yet available, not until server calls [ErrorsWrite.report]. *)
  | Complete of Telemetry.t
      (** The typecheck has finished, i.e. server called [ErrorsWrite.complete]. *)
  | Restarted
      (** The typecheck didn't complete; a new typecheck in a new errors-file has started.
        i.e. server called [ErrorsWrite.new_empty_file] before [ErrorsWrite.complete]. *)
  | Stopped
      (** Hh_server was stopped gracefully so we can't read errors. i.e. server called [ErrorsWrite.unlink]. *)
  | Killed  (** Hh_server was killed so we can't read errors. *)
  | Build_id_mismatch
      (** The hh_server that produced these errors is incompatible with the current binary. *)
[@@deriving show]

module ErrorsWrite : sig
  (** To be called at start of typechecking.
  This creates a new errors file. If there had been a previous errors file, then the previous
  one gets unlinked; and if the previous error file was not yet complete then anyone
  reading from the previous errors file will now get a [Error Restarted]. *)
  val new_empty_file :
    clock:Watchman.clock option -> ignore_hh_version:bool -> unit

  (** To be called during typechecking.
  Anyone reading the current errors file will get this error report as [Ok errors].
  This call will failwith if called before [new_empty_file], or after [complete]/[unlink_at_server_stop]. *)
  val report : Errors.t -> unit

  (** To be called at the end of typechecking.
  After this, anyone reading the errors file will get [Error Complete].
  This call will failwith if called before [new_empty_file] or after [complete]/[unlink_at_server_stop]. *)
  val complete : Telemetry.t -> unit

  (** To be called upon server shutdown, e.g. after "hh stop" or .hhconfig change.
  After this, anyone reading the errors file will get [Error Stopped]. *)
  val unlink_at_server_stop : unit -> unit

  (** Internal, for testing only. *)
  val get_state_FOR_TEST : unit -> string

  (** Internal, for testing only *)
  val create_file_FOR_TEST : pid:int -> cmdline:string -> unit
end

module ErrorsRead : sig
  (** A [log_message] is a string which should go to logs, but not be surfaced to users. *)
  type log_message = string

  (** A successful call to [openfile] returns this. *)
  type open_success = {
    pid: int;
        (** the PID of the server process that produced this errors file *)
    timestamp: float;  (** The time at which the typecheck started. *)
    clock: Watchman.clock option;
        (** The watchman clock at which this typecheck started. *)
  }

  (** [openfile fd] opens an error-file for reading, one that has been created
  through [ErrorsWrite.new_empty_file]. The only error conditions this can
  return are [Error Killed] or [Error Build_id_mismatch]. *)
  val openfile :
    Unix.file_descr -> (open_success, errors_file_error * log_message) result

  (** This is the return type for [read_next_errors]. In case of success, it includes
  a timestamp when they were reported. *)
  type read_result =
    ( Errors.error list Relative_path.Map.t * float,
      errors_file_error * log_message )
    result

  (** Attempt to get the next batch of errors. It returns based on a queue
  of what the server was written to the errors file...
  * For each time the server did [ErrorsWrite.report errors], this function will return [Ok (errors, timestamp)].
  * If the server hasn't yet done further [ErrorsWrite.report], this will return [Error NothingYet].
  * If the server did [ErrorsWrite.complete] then this will return [Error Complete].
  * If the server did [ErrorsWrite.new_empty_file] then this will return [Error Restarted].
  * If the server did [ErrorsWrite.unlink_at_server_stop] then this will return [Error Stopped].
  * If the server was killed, then this will return [Error Killed]. *)
  val read_next_errors : Unix.file_descr -> read_result
end
