(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let main env =
  HackEventLogger.client_restart ();
  if MonitorConnection.server_exists
  (ServerFiles.lock_file env.ClientStart.root) then
    ClientStop.kill_server env.ClientStart.root
  else Printf.eprintf "Warning: no server to restart for %s\n%!"
    (Path.to_string env.ClientStart.root);
  ClientStart.start_server env;
  Exit_status.No_error
