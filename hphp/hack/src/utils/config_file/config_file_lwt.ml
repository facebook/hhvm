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

let parse_hhconfig (fn : string) : (t, string) Lwt_result.t =
  let%lwt contents = Lwt_utils.read_all fn in
  match contents with
  | Ok hhconfig_contents ->
    let parsed = Config_file_common.parse_contents hhconfig_contents in
    Lwt.return_ok parsed
  | Error message ->
    Lwt.return_error (Printf.sprintf "Could not load hhconfig: %s" message)
