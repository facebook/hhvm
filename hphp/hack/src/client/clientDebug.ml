(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root: Path.t;
  from: string;
}

let main (env : env) : Exit_status.t Lwt.t =
  let%lwt { ClientConnect.channels = (ic, oc); _ } =
    ClientConnect.connect
      {
        ClientConnect.root = env.root;
        from = env.from;
        autostart = true;
        force_dormant_start = false;
        ai_mode = None;
        deadline = None;
        no_load = false;
        watchman_debug_logging = false;
        log_inference_constraints = false;
        profile_log = false;
        remote = false;
        progress_callback = ClientConnect.tty_progress_reporter ();
        do_post_handoff_handshake = true;
        ignore_hh_version = false;
        saved_state_ignore_hhconfig = false;
        use_priority_pipe = false;
        prechecked = None;
        config = [];
        allow_non_opt_build = false;
      }
  in
  ServerCommandLwt.connect_debug oc;

  (* Exit this via ctrl-C *)
  while true do
    print_endline (Timeout.input_line ic)
  done;
  Lwt.return Exit_status.No_error
