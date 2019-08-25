(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_json

let print_json res = print_endline (Hh_json.json_to_string (JSON_String res))

let go res output_json =
  if output_json then
    print_json res
  else
    print_endline res
