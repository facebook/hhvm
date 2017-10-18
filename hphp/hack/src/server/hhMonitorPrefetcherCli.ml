(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * This binary is strictly for manual testing since testing the prefetcher
 * in a unit test is too tedious to set up.
 *
 * This is never used for anything anywhere - only manual testing to
 * demonstrate that the State_prefetcher module does
 * what it claims to.
 *)

module Informant = HhMonitorInformant

module Args = struct

  type t = {
    svn_rev : int;
    prefetcher_script: Path.t
  }

  let usage = Printf.sprintf
    "Usage: %s --script <prefetcher script> --svn-rev <svn rev>\n" Sys.argv.(0)

  let usage_exit () =
    Printf.eprintf "%s\n" usage;
    exit 1

  let string_spec str_ref =
    Arg.String (fun x -> str_ref := Some x)

  let int_spec int_ref =
    Arg.Int (fun x -> int_ref := Some x)

  let requires name opt = match !opt with
    | None ->
      let () = Printf.eprintf "Missing required argument: %s\n" name in
      usage_exit ()
    | Some x -> x

  let parse () =
    let prefetcher_script = ref None in
    let svn_rev = ref None in
    let () = Arg.parse [
      "--script", (string_spec prefetcher_script), "Path to prefetcher script";
      "--svn-rev", (int_spec svn_rev), "The SVN Revision"]
      (fun _ -> ()) usage in
    let prefetcher_script = requires "prefetcher script" prefetcher_script in
    let svn_rev = requires "svn_rev" svn_rev in
    {
      svn_rev;
      prefetcher_script = Path.make prefetcher_script;
    }

end;;

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  let prefetcher = State_prefetcher.of_script_opt
    (Some args.Args.prefetcher_script) in
  let process = State_prefetcher.run
    args.Args.svn_rev prefetcher in
  let wrap_text_block str = Printf.sprintf
    "\n%s\n%s\n%s\n"
    "============================================="
    str
    "============================================="
  in
  match Process.read_and_wait_pid ~timeout:30 process with
  | Ok _ ->
    exit 0
  | Error Process_types.Process_aborted_input_too_large ->
    Printf.eprintf "Process_aborted_input_too_large\n";
    exit 1
  | Error (Process_types.Process_exited_abnormally (_, _, stderr))
  | Error (Process_types.Timed_out (_, stderr)) ->
    Printf.eprintf
      "Exited abnormally. See also stderr:%s" (wrap_text_block stderr);
    exit 1
