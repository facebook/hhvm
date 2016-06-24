(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

let print_json_legacy res =
  print_endline (Hh_json.json_to_string (FileOutline.to_json_legacy res));
  ()

let print_readable_legacy res =
  List.iter res begin fun (pos, name, type_) ->
    print_endline ((Pos.string pos)^" "^name^" ("^type_^")")
  end;
  ()

let go_legacy res output_json =
  let res = FileOutline.to_legacy res in
  if output_json then
    print_json_legacy res
  else
    print_readable_legacy res

let print_json res =
  print_endline (Hh_json.json_to_string (FileOutline.to_json res));
  ()

let go res output_json =
  if output_json then
    print_json res
  else
    FileOutline.print res

let print_json_definition res =
  let json = match res with
    | Some res -> FileOutline.definition_to_json res
    | None -> Hh_json.JSON_Null
  in
  print_endline (Hh_json.json_to_string json)

let print_readable_definition res =
  match res with
  | Some res -> FileOutline.print_def "" res
  | None -> print_endline "None"

let print_definition res output_json =
  if output_json then
    print_json_definition res
  else
    print_readable_definition res
