(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type version_components = {
  major: int;
  minor: int;
  build: int;
}

type version =
  | Opaque_version of string option
  | Version_components of version_components

let version_to_string_opt (v : version) : string option =
  match v with
  | Opaque_version s -> s
  | Version_components { major; minor; build } ->
    Some (Printf.sprintf "%d.%02d.%d" major minor build)

let version_re_str = {|\^\([0-9]+\)\.\([0-9]+\)\.\([0-9]+\)|}

let version_re = Str.regexp version_re_str

let parse_version (version : string option) =
  match version with
  | Some version when Str.string_match version_re version 0 ->
    begin
      try
        let major = int_of_string (Str.matched_group 1 version) in
        let minor = int_of_string (Str.matched_group 2 version) in
        let build = int_of_string (Str.matched_group 3 version) in
        Version_components { major; minor; build }
      with _ ->
        Hh_logger.log
          "Failed to parse server version '%s', treating it as an opaque string"
          version;
        Opaque_version (Some version)
    end
  | Some version ->
    Hh_logger.log
      "Server version '%s' does not match version pattern %s, treating it as an opaque string"
      version
      version_re_str;
    Opaque_version (Some version)
  | None ->
    Hh_logger.log "Server version is not set";
    Opaque_version None

let compare_versions v1 v2 =
  match (v1, v2) with
  | (Version_components c1, Version_components c2) -> compare c1 c2
  | (Opaque_version (Some s1), Opaque_version (Some s2)) -> compare s1 s2
  | (Opaque_version None, Opaque_version None) -> 0
  | (Opaque_version None, Opaque_version (Some _))
  | (Opaque_version _, Version_components _) ->
    -1
  | (Opaque_version (Some _), Opaque_version None)
  | (Version_components _, Opaque_version _) ->
    1
