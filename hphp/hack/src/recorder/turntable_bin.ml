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
 * Turntable plays back a recording on a Hack server. Upon finishing playback,
 * it sleeps until manually terminated, keeping the persistent connection
 * alive so that the server state doesn't drop the IDE state (allowing you to
 * to interact with and inspect the server).
 *)

module Args = struct

  type t = {
    root : Path.t;
    recording : Path.t;
  }

  let usage = Printf.sprintf
    "Usage: %s --recording <recording file> [REPO DIRECTORY]\n"
    Sys.argv.(0)

  let parse () =
    let root = ref None in
    let recording = ref "" in
    let options = [
      "--recording", Arg.String (fun x -> recording := x),
      "Path to the recording file";
    ] in
    let () = Arg.parse options (fun s -> root := (Some s)) usage in
    let root = ClientArgsUtils.get_root !root in
    if !recording = "" then
      let () = Printf.eprintf "%s" usage in
      exit 1
    else
    {
      root = root;
      recording = Path.make !recording;
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
  let () = Turntable.spin_record args.Args.recording (Args.root args) in
  let () = Printf.eprintf "End of recording...waiting for termination\n%!" in
  sleep_and_wait ()
