(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let print_json res =
  Nuclide_rpc_message_printer.print_json
    ~response:(Ide_message.Outline_response res)

let go res output_json =
  if output_json then
    print_json res
  else
    FileOutline.print res

let print_json_definition res =
  Nuclide_rpc_message_printer.print_json
    ~response:(Ide_message.Symbol_by_id_response res)

let print_readable_definition res =
  match res with
  | Some res -> FileOutline.print_def "" res
  | None -> print_endline "None"

let print_definition res output_json =
  if output_json then
    print_json_definition res
  else
    print_readable_definition res
