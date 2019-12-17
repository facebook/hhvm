(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Monotonically increasing identifier that can be used when we introduce
 * backward incompatible changes in hh_client commands, and to signal
 * new capabilities to clients.
 * v1 (hvvm 3.15, 11 May 2016) - persistent connection introduced
 * v4 (hvvm 3.18, 7 Nov 2016) - persistent connection stable
 * v5 (hvvm 3.23, 17 Nov 2017) - 'hh_client lsp' stable
 *)
let api_version = 5

let version : string =
  match Build_banner.banner with
  | Some banner -> banner
  | None -> Build_id.build_revision ^ " " ^ Build_id.build_commit_time_string

let version_json =
  Hh_json.(
    JSON_Object
      [
        ("commit", JSON_String Build_id.build_revision);
        ("commit_time", int_ Build_id.build_commit_time);
        ("build_mode", JSON_String Build_id.build_mode);
        ("api_version", int_ api_version);
      ])
