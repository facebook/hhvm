(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_json

let to_json result =
  let result = match result with
    | Ok () -> "result", (JSON_String "ok")
    | Error s -> "error_message", JSON_String s
  in
  JSON_Object [ result ]

let print_json res =
  print_endline (Hh_json.json_to_string (to_json res))

let print_readable = function
  | Ok () -> ()
  | Error s -> print_endline (Printf.sprintf "Error: %s" s)

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res
