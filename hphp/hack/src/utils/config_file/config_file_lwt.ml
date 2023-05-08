(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
include Config_file_version

type t = Config_file_common.t

let file_path_relative_to_repo_root =
  Config_file_common.file_path_relative_to_repo_root

let empty = Config_file_common.empty

let read_package_config (fn : string) : (string, string) Lwt_result.t =
  let%lwt contents = Lwt_utils.read_all fn in
  match contents with
  | Ok c -> Lwt.return_ok c
  | Error _ -> Lwt.return_ok "" (* If file doesn't exist return empty string *)

let parse_hhconfig (fn : string) : (string * t, string) Lwt_result.t =
  let%lwt contents = Lwt_utils.read_all fn in
  match contents with
  | Ok contents ->
    let parsed = Config_file_common.parse_contents contents in
    let package_path =
      Config_file_common.get_packages_absolute_path ~hhconfig_path:fn
    in
    let%lwt extra_config = read_package_config package_path in
    (match extra_config with
    | Ok extra_config ->
      let hash = Sha1.digest (contents ^ extra_config) in
      Lwt.return_ok (hash, parsed)
    | Error _ ->
      let hash = Sha1.digest contents in
      Lwt.return_ok (hash, parsed))
  | Error message ->
    Lwt.return_error (Printf.sprintf "Could not load hhconfig: %s" message)
