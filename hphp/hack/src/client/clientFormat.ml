(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_json

let to_json result =
  let error, result, internal_error = match result with
    | Ok s -> "", s, false
    | Error s -> s, "", true
  in
  JSON_Object [
    "error_message",  JSON_String error;
    "result",         JSON_String result;
    "internal_error", JSON_Bool internal_error;
  ]

let print_json res =
  print_endline (Hh_json.json_to_string (to_json res))

let print_readable = function
  | Ok res -> print_string res
  | _ -> ()

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res
