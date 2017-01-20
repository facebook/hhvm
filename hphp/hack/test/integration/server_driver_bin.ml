(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * Does a bunch of things to a directory and/or a Hack server running on that
 * directory such as deleting/modifying files on disk, connecting to the server
 * and making requests, reading results from those requests, etc. We need this
 * to automate things for integration testing when we want to avoid python
 * (since IDE requests would be strings in python making integration tests
 * brittle and hard to update when the API changes), and when we can't use
 * integration_ml test because we need to interact with a server that is
 * "more real" than the one offered there.
 *)

module Args = struct

  type t = {
    root : Path.t;
    test_case : int;
  }

  let usage = Printf.sprintf "Usage: %s [WWW DIRECTORY]\n" Sys.argv.(0)

  let parse () =
    let root = ref None in
    let test_case = ref 0 in
    let options = [
      "--test-case", Arg.Int (fun x -> test_case := x),
      "The test case to run. See also cmd_responses_case_[n]";
    ] in
    let () = Arg.parse options (fun s -> root := (Some s)) usage in
    let root = ClientArgsUtils.get_root !root in
    {
      root = root;
      test_case = !test_case;
    }

  let root args = args.root

end;;


let rec sleep_and_wait () =
  let _, _, _ = Unix.select [] [] [] 999999.999 in
  sleep_and_wait ()

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  Sys_utils.set_signal Sys.sigint (Sys.Signal_handle (fun _ ->
    exit 0));
  let args = Args.parse () in
  HackEventLogger.client_init @@ Args.root args;
  let _ = Server_driver.connect_and_run_case
    args.Args.test_case (Args.root args) in
  Printf.eprintf "Finished\n%!";
  sleep_and_wait ()
