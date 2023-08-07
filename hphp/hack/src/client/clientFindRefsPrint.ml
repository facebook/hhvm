(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let print_result (name, pos) =
  let pos_str = Pos.string pos in
  print_endline (pos_str ^ " " ^ name);
  ()

let print_json res =
  print_endline (Hh_json.json_to_string (ServerFindRefs.to_json res))

let print_readable res =
  List.iter res ~f:print_result;
  print_endline (string_of_int (List.length res) ^ " total results")

(* Versions used by the editor's "references to symbol at position" command *)
let print_ide_json res =
  let response = FindRefsService.result_to_ide_message res in
  Nuclide_rpc_message_printer.(
    find_references_response_to_json response |> print_json)

let print_ide_readable results =
  List.iter (List.rev results) ~f:(fun (_name, p) ->
      Printf.printf "%s\n" (Pos.string p));
  print_endline (string_of_int (List.length results) ^ " total results")

let print
    (res : ServerCommandTypes.Find_refs.result) ~(ide : bool) ~(json : bool) =
  match (json, ide) with
  | (true, false) -> print_json res
  | (true, true) -> print_ide_json res
  | (false, false) -> print_readable res
  | (false, true) -> print_ide_readable res
