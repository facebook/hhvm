(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
  | Error s ->
    let msg = Printf.sprintf "Error: %s" s in
    print_endline msg;
    exit 1

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res
