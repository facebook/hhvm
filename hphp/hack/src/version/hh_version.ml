(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let version : string =
  match Build_banner.banner with
  | Some banner -> banner
  | None -> Build_id.build_revision ^ " " ^ Build_id.build_commit_time_string

let version_json =
  let open Hh_json in
  JSON_Object [
    "commit", JSON_String Build_id.build_revision;
    "commit_time", int_ Build_id.build_commit_time;
    "api_version", int_ Build_id.build_api_version;
  ]
