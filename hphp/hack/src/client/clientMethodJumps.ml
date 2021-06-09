(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module MethodJumps = ServerCommandTypes.Method_jumps

let pos_to_json pos =
  let (line, start, end_) = Pos.info_pos pos in
  Hh_json.JSON_Object
    [
      ("file", Hh_json.JSON_String (Pos.filename pos));
      (* we can't use Pos.json *)
      ("line", Hh_json.int_ line);
      ("char_start", Hh_json.int_ start);
      ("char_end", Hh_json.int_ end_);
    ]

let cls_or_mthd_to_json name pos p_name =
  Hh_json.JSON_Object
    [
      ("name", Hh_json.JSON_String (Utils.strip_ns name));
      ("pos", pos_to_json pos);
      ("parent_name", Hh_json.JSON_String (Utils.strip_ns p_name));
    ]

let to_json input =
  let entries =
    List.map input ~f:(fun res ->
        Hh_json.JSON_Object
          [
            ( "origin",
              cls_or_mthd_to_json
                res.MethodJumps.orig_name
                res.MethodJumps.orig_pos
                res.MethodJumps.orig_p_name );
            ( "destination",
              cls_or_mthd_to_json
                res.MethodJumps.dest_name
                res.MethodJumps.dest_pos
                res.MethodJumps.dest_p_name );
          ])
  in
  Hh_json.JSON_Array entries

let print_json res =
  print_endline (Hh_json.json_to_string (to_json res));
  ()

let go res find_children output_json =
  if output_json then
    print_json res
  else
    MethodJumps.print_readable res ~find_children
