(**
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

(*
 * Config file format:
 * # Some comment. Indicate by a pound sign at the start of a new line
 * key = a possibly space-separated value
 *)
let parse_contents contents =
  let lines = Str.split (Str.regexp "\n") contents in
  List.fold_left lines ~f:begin fun acc line ->
    if String.strip line = "" || (String.length line > 0 && line.[0] = '#')
    then acc
    else
      let parts = Str.bounded_split (Str.regexp "=") line 2 in
      match parts with
      | [k; v] -> SMap.add (String.strip k) (String.strip v) acc
      | [k] -> SMap.add (String.strip k) "" acc
      | _ -> failwith "failed to parse config";
  end ~init:SMap.empty

let parse fn =
  let contents = try cat fn
    with e ->
      let stack = Printexc.get_backtrace () in
      Hh_logger.exc ~prefix:".hhconfig deleted: " ~stack e;
      Exit_status.(exit Hhconfig_deleted) in
  let parsed = parse_contents contents in
  let hash = Sha1.digest contents in
  hash, parsed

module Getters = struct

  let string_opt key config =
    SMap.get key config

  let string_ key ~default config =
    Option.value (SMap.get key config) ~default

  let int_ key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:int_of_string

  let int_opt key config =
    Option.map (SMap.get key config) ~f:int_of_string

  let float_ key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:float_of_string

  let float_opt key config =
    Option.map (SMap.get key config) ~f:float_of_string

  let bool_ key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:bool_of_string

  let bool_opt key config =
    Option.map (SMap.get key config) ~f:bool_of_string

  let string_list ~delim key ~default config =
    Option.value_map (SMap.get key config) ~default ~f:(Str.split delim)
  (*
    Version aware toggling for boolean parameters:
    take in either:
    - a single boolean, true or false. Then this acts just like bool_
    - a string list of version hashes, e.g.
       use_watchman = 8a0f4290c3fd218988864e06b7dea3aaf447efaf, master
      this would enable watchman for hack versions
        8a0f4290c3fd218988864e06b7dea3aaf447efaf and master.
      master is a special string to represent master in fbcode
      (master's build revision is the empty string, but master is a lot \
       cleaner to write)
  *)
  let bool_if_version key ~default config =
      let versions = string_list ~delim:(Str.regexp ",")
        key ~default:([string_of_bool default]) config in
      match versions with
      | ["true"] -> true
      | ["false"] -> false
      | x -> List.exists ~f:(fun s ->
          let s = if s = "master" then "" else s in
          s = Build_id.build_revision) x
end
