(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Test runner for list analysis orchestrator end-to-end tests.

    Each test case is a fixture directory containing:
    - .hhconfig (possibly empty)
    - *.php files with list() expressions

    The runner copies the fixture to a temp directory, runs the orchestrator
    with --quiet, normalizes paths in the output, and prints the summary.

    Invoked by verify.py as: test_runner <config_filename> <flags...>
    where flags = orchestrator hh_distc hh_distc_worker list_logger_summary *)

open Hh_prelude

let run_command ?(stderr_handling = "2>&1") (cmd : string) : string * int =
  let full_cmd = cmd ^ " " ^ stderr_handling in
  let ic = Unix.open_process_in full_cmd in
  let buf = Buffer.create 4096 in
  (try
     while true do
       let line = In_channel.input_line_exn ic in
       Buffer.add_string buf line;
       Buffer.add_char buf '\n'
     done
   with
  | End_of_file -> ());
  let status = Unix.close_process_in ic in
  let exit_code =
    match status with
    | Unix.WEXITED n -> n
    | Unix.WSIGNALED n -> 128 + n
    | Unix.WSTOPPED n -> 128 + n
  in
  (Buffer.contents buf, exit_code)

let normalize_paths ~(tmp_dir : string) (s : string) : string =
  (* Replace all occurrences of paths under tmp_dir with ROOT.
     working_dir is a subdirectory of tmp_dir, so this single pass
     handles both cases. *)
  let len = String.length tmp_dir in
  let buf = Buffer.create (String.length s) in
  let i = ref 0 in
  while !i < String.length s do
    if
      !i + len <= String.length s
      && String.equal (String.sub s ~pos:!i ~len) tmp_dir
    then begin
      Buffer.add_string buf "ROOT";
      i := !i + len;
      (* Skip rest of path characters (not whitespace, not ')') *)
      while
        !i < String.length s
        && (not (Char.is_whitespace s.[!i]))
        && not (Char.equal s.[!i] ')')
      do
        i := !i + 1
      done
    end else begin
      Buffer.add_char buf s.[!i];
      i := !i + 1
    end
  done;
  Buffer.contents buf

let () =
  if Array.length Sys.argv < 6 then begin
    Printf.eprintf
      "Usage: %s <config_filename> <orchestrator> <hh_distc> <hh_distc_worker> <list_logger_summary>\n"
      Sys.argv.(0);
    exit 1
  end;
  (* verify.py sets cwd to the test fixture directory and passes the config
     filename as the first arg. We use cwd directly. *)
  let _config_filename = Sys.argv.(1) in
  let orchestrator = Sys.argv.(2) in
  let hh_distc = Sys.argv.(3) in
  let hh_distc_worker = Sys.argv.(4) in
  let list_logger_summary = Sys.argv.(5) in

  let test_dir = Disk.getcwd () in
  let tmp_dir = Core_unix.mkdtemp "/tmp/list_analysis_test_XXXXXX" in

  (* Register cleanup so temp files are removed even on abnormal exit *)
  Stdlib.at_exit (fun () ->
      ignore (Sys.command (Printf.sprintf "rm -rf %s" (Filename.quote tmp_dir))));

  let working_dir = Filename.concat tmp_dir "root" in
  let cp_exit =
    Sys.command
      (Printf.sprintf
         "cp -r %s %s"
         (Filename.quote test_dir)
         (Filename.quote working_dir))
  in
  if cp_exit <> 0 then begin
    Printf.eprintf "Failed to copy test directory\n";
    exit 1
  end;

  let cmd =
    Printf.sprintf
      "%s --quiet --hh-distc=%s --hh-distc-worker=%s --list-logger-summary=%s --root %s"
      (Filename.quote orchestrator)
      (Filename.quote hh_distc)
      (Filename.quote hh_distc_worker)
      (Filename.quote list_logger_summary)
      (Filename.quote working_dir)
  in
  let (output, exit_code) = run_command ~stderr_handling:"2>/dev/null" cmd in
  let output =
    if exit_code <> 0 && exit_code <> 2 then
      (* exit code 2 = type errors found, which is expected *)
      if String.is_empty (String.strip output) then
        (* Re-run capturing stderr to get error details *)
        let (all_output, _) = run_command cmd in
        if String.is_empty (String.strip all_output) then
          Printf.sprintf "Exit code %d" exit_code
        else
          all_output
      else
        output
    else
      output
  in
  let output = normalize_paths ~tmp_dir output in
  Out_channel.output_string Out_channel.stdout output
