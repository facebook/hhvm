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

let parse_hhconfig (fn : string) : (string * string SMap.t, string) Lwt_result.t
    =
  let%lwt contents = Lwt_utils.read_all fn in
  match contents with
  | Ok contents ->
    let parsed = Config_file_common.parse_contents contents in
    let hash = Sha1.digest contents in
    Lwt.return_ok (hash, parsed)
  | Error message ->
    Lwt.return_error (Printf.sprintf "Could not load hhconfig: %s" message)
