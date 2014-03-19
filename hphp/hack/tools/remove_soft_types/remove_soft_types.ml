(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Common

type mode = Harden | DeleteFromLog

let parse_options () =
  let root = ref "" in
  let harden = ref false in
  let delete_from_log = ref false in

  let switch x = Arg.Unit (fun () -> x := true) in

  let usage = Printf.sprintf
    "Usage: %s [--harden|--delete-from-log] file-or-logfile\n"
    Sys.argv.(0) in

  let options = [
    "--harden", switch harden,
    " Transform all the soft typehints into hard typehints for the given file.";

    "--delete-from-log", switch delete_from_log,
    " Read the given HPHP log file and delete the soft typehints which aren't
     passing.";
  ] in
  let options = Arg.align options in

  Arg.parse options (fun s -> root := s) usage;

  if !root = "" then begin
    prerr_endline "Must specify a file to operate on";
    prerr_endline usage;
    exit 1
  end;

  if (!harden && !delete_from_log) || (not !harden && not !delete_from_log)
  then begin
    prerr_endline "Must specify exactly one of --harden or --delete-from-log";
    prerr_endline usage;
    exit 1
  end;

  (if !harden then Harden else DeleteFromLog), !root

let main () =
  let mode, root = parse_options () in
  match mode with
    | DeleteFromLog -> Delete_from_log.go root
    | Harden -> Harden.go root

let _ =
  Common.main_boilerplate (fun () -> ignore (main ()))
