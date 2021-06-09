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
    identify_symbol_response_to_json res |> print_json)

let print_readable ?(short_pos = false) x =
  List.iter x ~f:(function (occurrence, definition) ->
      SymbolOccurrence.(
        let { name; type_; pos; is_declaration } = occurrence in
        Printf.printf
          "name: %s, kind: %s, span: %s, is_declaration: %b\n"
          name
          (kind_to_string type_)
          (Pos.string_no_file pos)
          is_declaration;
        Printf.printf "definition:";
        begin
          match definition with
          | None -> Printf.printf " None\n"
          | Some definition ->
            Out_channel.newline stdout;
            FileOutline.print_def ~short_pos " " definition
        end;
        Out_channel.newline stdout))

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res
