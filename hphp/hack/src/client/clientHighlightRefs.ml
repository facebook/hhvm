(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open Hh_json

let to_json res =
  JSON_Array (List.map res (Pos.json))

let print_json res =
  let response = Ide_message.Highlight_references_response res in
  Nuclide_rpc_message_printer.print_json ~response

let print_result pos =
  Printf.printf "%s\n" (Ide_api_types.range_to_string_single_line pos)

let print_readable res =
  List.iter res print_result;
  print_endline ((string_of_int (List.length res)) ^ " total results")

let go res ~output_json =
  if output_json then
    print_json res
  else
    print_readable res
