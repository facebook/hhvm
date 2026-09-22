(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module MethodJumps = Server_command_types.Method_jumps

let pos_to_json pos =
  let (line, start, end_) = Pos.info_pos pos in
  `Assoc
    [
      ("file", `String (Pos.filename pos));
      (* we can't use Pos.json *)
      ("line", `Int line);
      ("char_start", `Int start);
      ("char_end", `Int end_);
    ]

let cls_or_mthd_to_json name pos p_name =
  `Assoc
    [
      ("name", `String (Utils.strip_ns name));
      ("pos", pos_to_json pos);
      ("parent_name", `String (Utils.strip_ns p_name));
    ]

let to_json input =
  let entries =
    List.map input ~f:(fun res ->
        `Assoc
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
  `List entries

let print_json res =
  print_endline (Yojson.Safe.to_string (to_json res));
  ()

let go res find_children output_json =
  if output_json then
    print_json res
  else
    MethodJumps.print_readable res ~find_children
