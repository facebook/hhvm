(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let write path contents = Real_disk.write_file ~file:path ~contents

(** Install [script] as a fake [sl], recording its PID and pattern file for
    cleanup assertions. The script can use [$root] for the fixture directory.
    [None] makes launch fail. *)
let with_fake_sl ~script f =
  Tempfile.with_real_tempdir (fun temp_dir ->
      let root = Path.to_string temp_dir in
      let bin = Filename.concat root "bin" in
      Real_disk.mkdir_p bin;
      Option.iter script ~f:(fun script ->
          let command = Filename.concat bin "sl" in
          write
            command
            ({|#!/bin/sh
# with_prefetch invokes sl with --cwd ROOT as its first two arguments.
root="$2"
# The include argument is listfile:PATH. Record PATH for cleanup assertions.
for arg do
  case "$arg" in
    listfile:*) printf '%s' "${arg#listfile:}" > "$root/pattern_file" ;;
  esac
done
# $$ is this shell's PID; the test checks that it disappears after cleanup.
printf '%s' "$$" > "$root/pid"
|}
            ^ script);
          Unix.chmod command 0o700);
      let previous_path = Sys.getenv "PATH" in
      (* Replace PATH so [None] cannot fall back to the real [sl]. The scripts
         use shell builtins because other commands are unavailable through
         this restricted PATH. *)
      Unix.putenv "PATH" bin;
      Utils.try_finally
        ~f:(fun () -> f ~root)
        ~finally:(fun () -> Unix.putenv "PATH" previous_path))

(** Poll fixture state with a deadline so a broken fixture fails the test. *)
let wait_for condition =
  let deadline = Unix.gettimeofday () +. 10.0 in
  while not (condition ()) do
    if Float.(Unix.gettimeofday () >= deadline) then
      failwith "Prefetch fixture did not become ready within ten seconds";
    Unix.sleepf 0.01
  done

let run ~root ~f =
  Edenfs_prefetch.with_prefetch
    ~root:(Path.make root)
    ~files:
      (Relative_path.Set.singleton (Relative_path.from_root ~suffix:"A.php"))
    ~workers:4
    ~f

(** Check whether the fake [sl] has exited without consuming its exit status. *)
let command_finished root =
  let pid_file = Filename.concat root "pid" in
  (* The script records its PID before running the per-test body. Until that
     file exists, we cannot check the command. *)
  if not (Sys.file_exists pid_file) then
    false
  else
    let pid = Sys_utils.cat pid_file |> int_of_string in
    let status_file = Printf.sprintf "/proc/%d/status" pid in
    (* On Linux, an absent /proc entry means the process is gone. State Z
       means it is a zombie: it has exited but has not yet been reaped. Both
       count as finished here. We must not use waitpid, since that would reap
       the child and consume the exit status that prefetch cleanup needs. *)
    (not (Sys.file_exists status_file))
    || String.is_substring (Sys_utils.cat status_file) ~substring:"State:\tZ"

(** Keep the foreground callback running until the fake [sl] exits naturally.
    Cleanup can then report its command outcome and timing. *)
let with_exited_prefetch ~root ~f =
  let child_exited = ref false in
  let previous_handler = ref Sys.Signal_default in
  (* Install the handler before launching sl, chain any previous handler, and
     restore it afterwards. This handler records notification without reaping. *)
  previous_handler :=
    Sys.signal
      Sys.sigchld
      (Sys.Signal_handle
         (fun signal ->
           child_exited := true;
           match !previous_handler with
           | Sys.Signal_handle handler -> handler signal
           | Sys.Signal_default
           | Sys.Signal_ignore ->
             ()));
  Utils.try_finally
    ~f:(fun () ->
      run ~root ~f:(fun () ->
          (* Linux can expose zombie state in /proc before notifying the parent
             that the child is ready to reap. Waiting for SIGCHLD as well avoids
             that race. SIGCHLD can refer to any child, so also check this PID. *)
          wait_for (fun () -> !child_exited && command_finished root);
          f ()))
    ~finally:(fun () -> Sys.set_signal Sys.sigchld !previous_handler)

(** Check that cleanup reaped the command and removed its pattern file. *)
let assert_command_reaped root =
  let pid = Sys_utils.cat (Filename.concat root "pid") |> int_of_string in
  (* A zombie still has a /proc entry. Unlike command_finished, this assertion
     requires the child to have been reaped. *)
  assert (not (Sys.file_exists (Printf.sprintf "/proc/%d" pid)));
  let pattern_file = Sys_utils.cat (Filename.concat root "pattern_file") in
  assert (not (Sys.file_exists pattern_file))

let telemetry_field telemetry key =
  match Telemetry.to_yojson telemetry with
  | `Assoc fields -> List.Assoc.find fields ~equal:String.equal key
  | _ -> assert false

(** A nonzero prefetch exit preserves foreground work and cleans up resources.
    Valid timing is reported; missing or malformed output must omit duration. *)
let test_command_failure_is_nonfatal () =
  List.iter
    [
      ("time: real 0.125 secs (user 0.000+0.000 sys 0.000+0.000)\n", Some 0.125);
      ("time: real invalid secs\n", None);
      ("", None);
    ]
    ~f:(fun (timing, expected_duration) ->
      (* Print the timing text to stderr, then exit unsuccessfully.
         Filename.quote preserves it as one literal shell argument;
         %%s becomes the shell printf's %s format. *)
      let script =
        Printf.sprintf "printf '%%s' %s >&2\nexit 7\n" (Filename.quote timing)
      in
      with_fake_sl ~script:(Some script) (fun ~root ->
          let log = Filename.concat root "prefetch.log" in
          Hh_logger.set_log log;
          let (result, telemetry) =
            with_exited_prefetch ~root ~f:(fun () -> 42)
          in
          assert (result = 42);
          assert (
            Poly.equal
              (telemetry_field telemetry "outcome")
              (Some (`String "command_failed")));
          assert (
            Poly.equal
              (telemetry_field telemetry "cleanup_failed")
              (Some (`Bool false)));
          assert (
            Poly.equal
              (telemetry_field telemetry "duration_sec")
              (Option.map expected_duration ~f:(fun seconds -> `Float seconds)));
          assert (
            Poly.equal
              (telemetry_field telemetry "duration_source")
              (Option.map expected_duration ~f:(fun _ -> `String "sl")));
          let contents = Sys_utils.cat log in
          assert (
            String.is_substring
              contents
              ~substring:"outcome=command_failed cleanup_failed=false");
          assert (
            String.is_substring
              contents
              ~substring:"EdenFS prefetch command failed:");
          assert_command_reaped root));
  true

(** Cleanup failure must preserve the command's outcome, including when both
    command execution and cleanup fail. The child must still be reaped. *)
let test_cleanup_failure_preserves_command_outcome () =
  List.iter
    [(0, "completed"); (7, "command_failed")]
    ~f:(fun (exit_code, expected_outcome) ->
      let script = Printf.sprintf "exit %d\n" exit_code in
      with_fake_sl ~script:(Some script) (fun ~root ->
          let log = Filename.concat root "prefetch.log" in
          Hh_logger.set_log log;
          let (result, telemetry) =
            with_exited_prefetch ~root ~f:(fun () ->
                (* Removing the file first makes the cleanup's unlink fail. *)
                let pattern_file =
                  Sys_utils.cat (Filename.concat root "pattern_file")
                in
                Sys.remove pattern_file;
                42)
          in
          assert (result = 42);
          assert (
            Poly.equal
              (telemetry_field telemetry "outcome")
              (Some (`String expected_outcome)));
          assert (
            Poly.equal
              (telemetry_field telemetry "cleanup_failed")
              (Some (`Bool true)));
          assert (
            String.is_substring
              (Sys_utils.cat log)
              ~substring:"EdenFS prefetch cleanup failed:");
          assert_command_reaped root));
  true

(** Failure to launch [sl] must preserve foreground work and remove the pattern
    file that was created before launch. *)
let test_launch_failure_is_nonfatal () =
  with_fake_sl ~script:None (fun ~root ->
      (* Isolate temp files so an empty directory proves there was no leak. *)
      let pattern_dir = Filename.concat root "patterns" in
      Real_disk.mkdir_p pattern_dir;
      let previous_temp_dir = Stdlib.Filename.get_temp_dir_name () in
      Stdlib.Filename.set_temp_dir_name pattern_dir;
      Utils.try_finally
        ~f:(fun () ->
          let log = Filename.concat root "prefetch.log" in
          Hh_logger.set_log log;
          let (result, telemetry) = run ~root ~f:(fun () -> 42) in
          assert (result = 42);
          assert (
            Poly.equal
              (telemetry_field telemetry "outcome")
              (Some (`String "launch_failed")));
          assert (
            Poly.equal
              (telemetry_field telemetry "cleanup_failed")
              (Some (`Bool false)));
          assert (Option.is_none (telemetry_field telemetry "duration_sec"));
          assert (
            String.is_substring
              (Sys_utils.cat log)
              ~substring:"EdenFS prefetch launch failed:");
          assert (Array.is_empty (Sys.readdir pattern_dir));
          true)
        ~finally:(fun () -> Stdlib.Filename.set_temp_dir_name previous_temp_dir))

(** Block in a shell builtin until SIGTERM, without spawning a child process. *)
let blocking_sl =
  {|# The empty marker proves the SIGTERM handler ran before the shell exited.
trap ': > "$root/terminated"; exit 0' TERM
# Open the fixture's FIFO on descriptor 3 in this shell. Opening both ends
# avoids waiting for another process to open it and prevents read from seeing EOF.
exec 3<> "$root/wait"
# Let the OCaml callback proceed only once the handler and FIFO are ready.
: > "$root/started"
# Nothing writes to the FIFO, so this builtin waits until SIGTERM runs the trap.
read -r ignored <&3
|}

(** Run [f] while the fake [sl] waits for termination. *)
let with_blocking_prefetch ~root ~f =
  Unix.mkfifo (Filename.concat root "wait") 0o600;
  run ~root ~f:(fun () ->
      (* The startup marker confirms that the SIGTERM handler is installed.
         Waiting here prevents cleanup from killing the shell before it is
         ready to record the signal. *)
      wait_for (fun () -> Sys.file_exists (Filename.concat root "started"));
      f ())

(** Returning from foreground work must stop and reap a still-running prefetch
    process, allow its SIGTERM handler to run, remove its pattern file, and
    preserve the return value. *)
let test_cleanup_on_foreground_return () =
  with_fake_sl ~script:(Some blocking_sl) (fun ~root ->
      let (result, telemetry) =
        with_blocking_prefetch ~root ~f:(fun () -> 42)
      in
      assert (result = 42);
      assert (
        Poly.equal
          (telemetry_field telemetry "outcome")
          (Some (`String "stopped")));
      assert (
        Poly.equal
          (telemetry_field telemetry "cleanup_failed")
          (Some (`Bool false)));
      assert (
        Poly.equal
          (telemetry_field telemetry "duration_source")
          (Some (`String "controller")));
      (match telemetry_field telemetry "duration_sec" with
      | Some (`Float seconds) -> assert (Float.(seconds >= 0.0))
      | _ -> assert false);
      assert (Sys.file_exists (Filename.concat root "terminated"));
      assert_command_reaped root;
      true)

exception Foreground_failure

(** A foreground exception must survive cleanup unchanged, with no leaked
    prefetch command or temporary pattern file. *)
let test_controller_exception () =
  with_fake_sl ~script:(Some blocking_sl) (fun ~root ->
      let raised =
        try
          ignore
            (with_blocking_prefetch ~root ~f:(fun () ->
                 raise Foreground_failure));
          false
        with
        | Foreground_failure -> true
      in
      assert raised;
      assert_command_reaped root;
      true)

let () =
  Unit_test.run_all
    [
      ("command_failure_is_nonfatal", test_command_failure_is_nonfatal);
      ( "cleanup_failure_preserves_command_outcome",
        test_cleanup_failure_preserves_command_outcome );
      ("launch_failure_is_nonfatal", test_launch_failure_is_nonfatal);
      ("cleanup_on_foreground_return", test_cleanup_on_foreground_return);
      ("controller_exception", test_controller_exception);
    ]
