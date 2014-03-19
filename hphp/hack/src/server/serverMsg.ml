(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(**
 * Commands and responses hh_server deals with
 *
 * This code is responsible for serializing/deserializing messages.
 *
 * It doesn't really matter if the code is clean or not. The only
 * thing to keep in mind is that the code must remain backwards compatible since
 * old clients end up hitting new servers during updates.
 *)

type build_opts = {
  root: Path.path;
  steps: string list option; (* steps for hack build to run.
                                 None means 'all' *)
  serial: bool; (* when true, don't use parallel workers *)
  test_dir: string option; (* test dir to generate into *)
  grade: bool; (* when true, diff test output against www and print
                  some stats *)
  list_classes: bool; (* when true, generate class list files for
                         traversed classes *)
  clean: bool; (* when true, do a clean build *)
  run_scripts: bool; (* when true, run remaining arc build steps
                     that we haven't figured out how to port yet*)
  verbose: bool;
}


type find_refs_action =
| Class of string
| Method of string * string
| Function of string

type command =
| ERROR_OUT_OF_DATE
| PRINT_TYPES of string
| STATUS of Path.path
| SKIP
| LIST_FILES
| AUTOCOMPLETE of string
| SAVE_STATE of string
| SHOW of string
| KILL
| PING
| BUILD of build_opts
| PROLOG
| FIND_REFS of find_refs_action
| IDENTIFY_FUNCTION of string * int * int
| OUTLINE of string
| INFER_TYPE of string * int * int (* filename, line, char *)

let cmd_to_channel (oc:out_channel) (cmd:command): unit =
  Printf.fprintf oc "%s\n" Build_id.build_id_ohai;
  Marshal.to_channel oc cmd [];
  flush oc

let cmd_from_channel (ic:in_channel): command =
  let s = input_line ic in
  if s <> Build_id.build_id_ohai
  then ERROR_OUT_OF_DATE
  else Marshal.from_channel ic

type directory_mismatch = {
  server: Path.path;
  client: Path.path;
}

type response =
| SERVER_OUT_OF_DATE
| DIRECTORY_MISMATCH of directory_mismatch
| NO_ERRORS
| ERRORS of Utils.error list
| SERVER_DYING
| PONG

let response_to_string = function
  | SERVER_OUT_OF_DATE -> "Server Out of Date"
  | DIRECTORY_MISMATCH _ -> "Directory Mismatch"
  | NO_ERRORS -> "No Errors"
  | ERRORS _ -> "Some Errors"
  | SERVER_DYING -> "Server Dying"
  | PONG -> "Pong"

let response_to_channel (oc:out_channel) (cmd:response): unit =
  Printf.fprintf oc "%s\n" Build_id.build_id_ohai;
  Marshal.to_channel oc cmd [];
  flush oc

let response_from_channel (ic:in_channel): response =
  let s = input_line ic in
  if s <> Build_id.build_id_ohai
  then SERVER_OUT_OF_DATE
  else Marshal.from_channel ic
