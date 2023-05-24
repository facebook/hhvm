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

let pkgs_config_path_relative_to_repo_root =
  Config_file_common.pkgs_config_path_relative_to_repo_root

let empty = Config_file_common.empty

let print_to_stderr = Config_file_common.print_to_stderr

let apply_overrides = Config_file_common.apply_overrides

let parse_contents = Config_file_common.parse_contents

module Getters = Config_file_common.Getters

let cat_packages_file (fn : string) =
  if Disk.file_exists fn then (
    try Sys_utils.cat fn with
    | exn ->
      let e = Exception.wrap exn in
      Hh_logger.exception_ ~prefix:"PACKAGES.toml deleted: " e;
      ""
  ) else
    ""

let parse_hhconfig (fn : string) : string * t =
  let contents =
    try Sys_utils.cat fn with
    | exn ->
      let e = Exception.wrap exn in
      Hh_logger.exception_ ~prefix:".hhconfig deleted: " e;
      Exit.exit Exit_status.Hhconfig_deleted
  in
  let parsed = Config_file_common.parse_contents contents in
  (* Take the PACKAGES.toml in the same directory as the .hhconfig *)
  let package_path =
    Config_file_common.get_packages_absolute_path ~hhconfig_path:fn
  in
  let package_config = Some (cat_packages_file package_path) in
  let hash =
    Config_file_common.hash ~config_contents:contents ~package_config
  in
  (hash, parsed)

let parse_local_config = Config_file_common.parse_local_config

let of_list = Config_file_common.of_list

let keys = Config_file_common.keys

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
