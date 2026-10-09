(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open String_utils

let relative_path path =
  let cwd = Filename.concat (Sys.getcwd ()) "" in
  lstrip path cwd

let format_message ((pos, message) : Pos.absolute * string) : string =
  let (start_line, start_column, end_line, end_column_exclusive) =
    Pos.destruct_range_one_based pos
  in
  let end_column =
    if Pos.length pos = 0 then
      start_column
    else
      end_column_exclusive - 1
  in
  Printf.sprintf
    "%s:%d:%d-%d:%d: %s"
    (relative_path (Pos.filename pos))
    start_line
    start_column
    end_line
    end_column
    message

let to_string (error : Diagnostics.finalized_diagnostic) : string =
  let { User_diagnostic.severity; code; claim; reasons; custom_msgs; _ } =
    error
  in
  let buf = Buffer.create 128 in
  Buffer.add_string
    buf
    (Printf.sprintf
       "%s: %s (%s)\n"
       (User_diagnostic.Severity.to_all_caps_string severity)
       (format_message claim)
       (User_diagnostic.error_code_to_string code));
  List.iter reasons ~f:(fun reason ->
      Buffer.add_string buf ("  " ^ format_message reason ^ "\n"));
  List.iter custom_msgs ~f:(fun message ->
      Buffer.add_string buf ("  " ^ message ^ "\n"));
  Buffer.add_char buf '\n';
  Buffer.contents buf
