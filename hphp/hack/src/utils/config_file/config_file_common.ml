(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Sys_utils

type t = string SMap.t

let file_path_relative_to_repo_root = ".hhconfig"

let print_config (config : string SMap.t) : unit =
  SMap.iter (fun k v -> Printf.eprintf "%s = %s\n" k v) config

let apply_overrides
    ~silent ~(config : string SMap.t) ~(overrides : string SMap.t) :
    string SMap.t =
  if SMap.cardinal overrides = 0 then
    config
  else
    (* Note that the order of arguments matters because SMap.union is left-biased by default. *)
    let config = SMap.union overrides config in
    if not silent then (
      Printf.eprintf "Config overrides:\n";
      print_config overrides;
      Printf.eprintf "\nThe combined config:\n";
      print_config config
    );
    config

(*
 * Config file format:
 * # Some comment. Indicate by a pound sign at the start of a new line
 * key = a possibly space-separated value
 *)
let parse_contents (contents : string) : string SMap.t =
  let lines = Str.split (Str.regexp "\n") contents in
  List.fold_left
    lines
    ~f:
      begin
        fun acc line ->
        if String.strip line = "" || (String.length line > 0 && line.[0] = '#')
        then
          acc
        else
          let parts = Str.bounded_split (Str.regexp "=") line 2 in
          match parts with
          | [k; v] -> SMap.add (String.strip k) (String.strip v) acc
          | [k] -> SMap.add (String.strip k) "" acc
          | _ -> failwith "failed to parse config"
      end
    ~init:SMap.empty

let parse ~silent (fn : string) : string * string SMap.t =
  let contents = cat fn in
  if not silent then
    Printf.eprintf "%s on-file-system contents:\n%s\n" fn contents;
  let parsed = parse_contents contents in
  let hash = Sha1.digest contents in
  (hash, parsed)

let parse_local_config ~silent (fn : string) : string SMap.t =
  try
    let (_hash, config) = parse ~silent fn in
    config
  with e ->
    Hh_logger.log "Loading config exception: %s" (Exn.to_string e);
    Hh_logger.log "Could not load config at %s" fn;
    SMap.empty

module Getters = struct
  let string_opt key config = SMap.get key config

  let string_ key ~default config = Option.value (SMap.get key config) ~default

  let int_ key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:int_of_string

  let int_opt key config = Option.map (SMap.get key config) ~f:int_of_string

  let float_ key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:float_of_string

  let float_opt key config =
    Option.map (SMap.get key config) ~f:float_of_string

  let bool_ key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:bool_of_string

  let bool_opt key config = Option.map (SMap.get key config) ~f:bool_of_string

  let string_list ~delim key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:(Str.split delim)

  (*
    Version aware toggling for boolean parameters take in either:
    - a single boolean, true or false. Then this acts just like bool_
    - a string list of version hashes, e.g.
       use_watchman = 8a0f4290c3fd218988864e06b7dea3aaf447efaf, master
      this would enable watchman for hack versions
        8a0f4290c3fd218988864e06b7dea3aaf447efaf and master.
      master is a special string to represent master in fbcode
      (master's build revision is the empty string, but master is a lot \
       cleaner to write)

    How this can be used:
      1) Commit a change that turns on a feature F if its option flag is set to "true"
      2) Discover that there's a bug in F and it needs to be turned off (set flag to "false")
      3) Fix the bug
      4) Wait for the next release to find out its build hash H1
      5) Update hh.conf in opsfiles to turn on the feature for "H1"
      6) Wait for the release after that to find out its build hash H2
      7) Update hh.conf in posfiles to turn the feature for "H1,H2"
      8) etc., until you're satisfied that nobody is using the release with the broken
          feature F, so you can change the flag's value back to "true"

    Note: it is the responsibility of the feature author to keep flag values up-to-date,
    not the Hack Tools or the Hack Release oncall's.
  *)
  let bool_if_version key ~default config =
    let versions =
      string_list
        ~delim:(Str.regexp ",")
        key
        ~default:[string_of_bool default]
        config
    in
    match versions with
    | ["true"] -> true
    | ["false"] -> false
    | x ->
      List.exists
        ~f:(fun s ->
          let s =
            if s = "master" then
              ""
            else
              s
          in
          s = Build_id.build_revision)
        x

  let bool_if_min_version key ~default ~current_version config =
    let version_value = string_ key ~default:(string_of_bool default) config in
    match version_value with
    | "true" -> true
    | "false" -> false
    | version_value ->
      let version_value =
        Config_file_version.parse_version (Some version_value)
      in
      if
        Config_file_version.compare_versions current_version version_value >= 0
      then
        true
      else
        false
end
