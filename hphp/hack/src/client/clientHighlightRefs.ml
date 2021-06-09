(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let print_json res =
  Nuclide_rpc_message_printer.(
    highlight_references_response_to_json res |> print_json)

let print_result pos =
  Printf.printf "%s\n" (Ide_api_types.range_to_string_single_line pos)

let print_readable res =
  List.iter res ~f:print_result;
  print_endline (string_of_int (List.length res) ^ " total results")

let go res ~output_json =
  if output_json then
    print_json res
  else
    print_readable res
