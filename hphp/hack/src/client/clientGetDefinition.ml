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

let print_json res =
  let response = IdentifySymbolService.result_to_ide_message res in
  Nuclide_rpc_message_printer.print_json ~response

let print_readable ?short_pos:(short_pos=false) x =
  List.iter x begin function (occurrence, definition) ->
    let open SymbolOccurrence in
    let {name; type_; pos;} = occurrence in
    Printf.printf "name: %s, kind: %s, span: %s, definition: "
      name (kind_to_string type_) (Pos.string_no_file pos);
    begin match definition with
    | None ->  Printf.printf "None\n"
    | Some definition ->
      print_newline ();
      FileOutline.print_def ~short_pos " " definition
    end;
    print_newline ()
  end

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res
