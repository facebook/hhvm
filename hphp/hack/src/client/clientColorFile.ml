(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

(*****************************************************************************)
(* Section defining the colors we are going to use *)
(*****************************************************************************)

module C = TtyColor
let err_clr       = C.Bold C.Red       (* Unchecked code *)
let checked_clr   = C.Normal C.Green   (* Checked code *)
let kwd_clr       = C.Normal C.Magenta (* Keyword *)
let fun_clr       = C.Normal C.Blue    (* Function name *)
let default_color = C.Normal C.Default (* All the rest *)

let replace_color input =
  match input with
  | (ColorFile.Unchecked_code, str) -> (err_clr, str)
  | (ColorFile.Checked_code, str) -> (checked_clr, str)
  | (ColorFile.Keyword, str) -> (kwd_clr, str)
  | (ColorFile.Fun, str) -> (fun_clr, str)
  | (ColorFile.Default, str) -> (default_color, str)

let replace_colors input =
  List.map replace_color input

let to_json input =
  let entries = List.map (fun (clr, text) ->
                          let color_string = match clr with
                          | ColorFile.Unchecked_code -> "err"
                          | ColorFile.Checked_code -> "checked"
                          | ColorFile.Keyword -> "kwd"
                          | ColorFile.Fun -> "fun"
                          | ColorFile.Default -> "default" in
                          Json.JAssoc [ "color", Json.JString color_string;
                                        "text",  Json.JString text;
                                      ]
                         ) input
  in
  Json.JList entries

(*****************************************************************************)
(* The entry point. *)
(*****************************************************************************)

let go filename output_json pos_type_l =
  let ic = open_in filename in
  let len = in_channel_length ic in
  let buf = Buffer.create len in
  Buffer.add_channel buf ic len;
  let str = Buffer.contents buf in
  let results = ColorFile.go str pos_type_l in
  if output_json then
    print_endline (Json.json_to_string (to_json results))
  else if Unix.isatty Unix.stdout
  then C.print (replace_colors results)
  else print_endline str
