(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
include Config_file_version

type t = string SMap.t

let file_path_relative_to_repo_root =
  Config_file_common.file_path_relative_to_repo_root

let apply_overrides = Config_file_common.apply_overrides

let parse_hhconfig ~silent (fn : string) : string * string SMap.t =
  try Config_file_common.parse ~silent fn
  with e ->
    let stack = Printexc.get_backtrace () in
    Hh_logger.exc ~prefix:".hhconfig deleted: " ~stack e;
    Exit.exit Exit_status.Hhconfig_deleted

let parse_local_config = Config_file_common.parse_local_config

module Getters = Config_file_common.Getters

module Utils = struct
  let parse_hhconfig_and_hh_conf_to_json
      ~(root : Path.t) ~(server_local_config_path : string) =
    let server_local_config =
      parse_local_config ~silent:true server_local_config_path
    in
    let hh_config =
      parse_local_config
        ~silent:true
        (Path.to_string (Path.concat root file_path_relative_to_repo_root))
    in
    let ssmap_to_json m =
      Hh_json.JSON_Object (SMap.elements @@ SMap.map Hh_json.string_ m)
    in
    Hh_json.JSON_Object
      [
        ("hh.conf", ssmap_to_json server_local_config);
        ("hhconfig", ssmap_to_json hh_config);
      ]
end
