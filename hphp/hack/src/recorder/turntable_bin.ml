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
    skip_hg_update_on_load_state : bool;
    recording : Path.t;
  }

  let usage = Printf.sprintf
    "Usage: %s --recording <recording file> %s [REPO DIRECTORY]\n"
    Sys.argv.(0)
    "[--skip-hg-update-on-load-state]"

  let parse () =
    let root = ref None in
    let recording = ref "" in
    let skip_hg_update_on_load_state = ref false in
    let options = [
      "--recording", Arg.String (fun x -> recording := x),
      "Path to the recording file";
      "--skip-hg-update-on-load-state", Arg.Set skip_hg_update_on_load_state,
      "When playing back load saved state, don't do an hg update";
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
      skip_hg_update_on_load_state = !skip_hg_update_on_load_state;
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
  let () = try Turntable.spin_record
    args.Args.skip_hg_update_on_load_state
    args.Args.recording
    (Args.root args) with
  | Exit_status.Exit_with e ->
    Printf.eprintf "Exit_with: %s\n" (Exit_status.to_string e);
    Exit_status.exit e
  in
  let () = Printf.printf "End of recording...waiting for termination\n%!" in
  sleep_and_wait ()
