(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let print_json res =
  Nuclide_rpc_message_printer.(outline_response_to_json res |> print_json)

let go res output_json =
  if output_json then
    print_json res
  else
    FileOutline.print res

let print_json_definition res =
  Nuclide_rpc_message_printer.(symbol_by_id_response_to_json res |> print_json)

let print_readable_definition res =
  match res with
  | Some res -> FileOutline.print_def "" res
  | None -> print_endline "None"
