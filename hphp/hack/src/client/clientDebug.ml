(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root : Path.t;
}

let main env =
  let {ClientConnect.channels = ic, oc; _} =
      ClientConnect.connect { ClientConnect.
    root = env.root;
    autostart = true;
    force_dormant_start = false;
    ai_mode = None;
    retries = Some 800;
    expiry = None;
    no_load = false;
    watchman_debug_logging = false;
    profile_log = false;
    progress_callback = ClientConnect.tty_progress_reporter ();
    do_post_handoff_handshake = true;
    ignore_hh_version = false;
    use_priority_pipe = false;
    prechecked = None;
  } in
  ServerCommand.connect_debug oc;
  (* Exit this via ctrl-C *)
  while true do
    print_endline (Timeout.input_line ic);
  done;
  Exit_status.No_error
