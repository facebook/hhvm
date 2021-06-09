(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open String_utils

let format_msg file_clr err_clr ((p, s) : Pos.absolute * string) =
  let (line, start, end_) = Pos.info_pos p in
  let file_path =
    if Unix.isatty Unix.stdout then
      let cwd = Filename.concat (Sys.getcwd ()) "" in
      lstrip (Pos.filename p) cwd
    else
      Pos.filename p
  in
  let line_clr = Tty.Normal Tty.Yellow in
  let col_clr = Tty.Normal Tty.Cyan in
  let default_clr = Tty.Normal Tty.Default in
  [
    (file_clr, file_path);
    (default_clr, ":");
    (line_clr, string_of_int line);
    (default_clr, ":");
    (col_clr, string_of_int start);
    (default_clr, ",");
    (col_clr, string_of_int end_);
    (default_clr, ": ");
    (err_clr, s);
  ]

let format_error_code code =
  [
    (Tty.Normal Tty.Default, " (");
    (Tty.Normal Tty.Yellow, Errors.error_code_to_string code);
    (Tty.Normal Tty.Default, ")");
  ]

let to_string (error : Errors.finalized_error) : string =
  let (error_code, msgl) = (Errors.get_code error, Errors.to_list error) in
  match msgl with
  | [] ->
    failwith "Impossible: an error always has a non-empty list of messages"
  | msg :: msgl ->
    let newline = (Tty.Normal Tty.Default, "\n") in
    let claim =
      format_msg (Tty.Bold Tty.Red) (Tty.Bold Tty.Red) msg
      @ format_error_code error_code
      @ [newline]
    in
    let reasons =
      let indent = (Tty.Normal Tty.Default, "  ") in
      List.concat_map
        ~f:(fun msg ->
          (indent :: format_msg (Tty.Normal Tty.Red) (Tty.Normal Tty.Green) msg)
          @ [newline])
        msgl
    in
    let to_print = claim @ reasons in
    if Unix.isatty Unix.stdout then
      List.map to_print ~f:(fun (c, s) -> Tty.apply_color c s) |> String.concat
    else
      List.map to_print ~f:(fun (_, x) -> x) |> String.concat
