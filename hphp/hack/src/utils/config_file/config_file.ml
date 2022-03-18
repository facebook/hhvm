(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Config_file_version

type t = Config_file_common.t

let file_path_relative_to_repo_root =
  Config_file_common.file_path_relative_to_repo_root

let empty = Config_file_common.empty

let print_to_stderr = Config_file_common.print_to_stderr

let apply_overrides = Config_file_common.apply_overrides

let parse_contents = Config_file_common.parse_contents

let parse_hhconfig (fn : string) : string * t =
  try Config_file_common.parse fn with
  | exn ->
    let e = Exception.wrap exn in
    Hh_logger.exception_ ~prefix:".hhconfig deleted: " e;
    Exit.exit Exit_status.Hhconfig_deleted

let parse_local_config = Config_file_common.parse_local_config

let of_list = Config_file_common.of_list

let keys = Config_file_common.keys

module Getters = Config_file_common.Getters

module Utils = struct
  let parse_hhconfig_and_hh_conf_to_json
      ~(root : Path.t) ~(server_local_config_path : string) =
    let server_local_config = parse_local_config server_local_config_path in
    let hh_config =
      parse_local_config
        (Path.to_string (Path.concat root file_path_relative_to_repo_root))
    in
    Hh_json.JSON_Object
      [
        ("hh.conf", Config_file_common.to_json server_local_config);
        ("hhconfig", Config_file_common.to_json hh_config);
      ]
end
