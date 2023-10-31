(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let main (env : ClientStart.env) : Exit_status.t Lwt.t =
  HackEventLogger.client_restart
    ~data:
      (Config_file.Utils.parse_hhconfig_and_hh_conf_to_json
         ~root:env.ClientStart.root
         ~server_local_config_path:ServerLocalConfig.system_config_path);
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
