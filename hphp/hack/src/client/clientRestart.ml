(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let main (env : ClientStart.env) : Exit_status.t Lwt.t =
  HackEventLogger.set_from env.ClientStart.from;
  HackEventLogger.client_restart ();
  if
    MonitorConnection.server_exists (ServerFiles.lock_file env.ClientStart.root)
  then
    ClientStop.kill_server env.ClientStart.root env.ClientStart.from
  else
    Printf.eprintf
      "Warning: no server to restart for %s\n%!"
      (Path.to_string env.ClientStart.root);
  ClientStart.start_server env;
  Lwt.return Exit_status.No_error
