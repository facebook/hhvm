(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let config_file_data env =
  let server_local_config =
    Config_file.parse_local_config ~silent:true ServerLocalConfig.path
  in
  let hh_config =
    Config_file.parse_local_config
      ~silent:true
      (Path.to_string
         (Path.concat
            env.ClientStart.root
            Config_file.file_path_relative_to_repo_root))
  in
  let ssmap_to_json m =
    Hh_json.JSON_Object (SMap.elements @@ SMap.map Hh_json.string_ m)
  in
  Hh_json.JSON_Object
    [
      ("hh.conf", ssmap_to_json server_local_config);
      ("hhconfig", ssmap_to_json hh_config);
    ]

let main (env : ClientStart.env) : Exit_status.t Lwt.t =
  HackEventLogger.set_from env.ClientStart.from;
  HackEventLogger.client_restart ~data:(config_file_data env);
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
