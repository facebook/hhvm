(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let notebook_to_hack ~(notebook_name : string) : Exit_status.t =
  let source = Sys_utils.read_stdin_to_string () in
  let ipynb_json = Hh_json.json_of_string source in
  match Notebook_to_hack.notebook_to_hack ~notebook_name ipynb_json with
  | Ok hack ->
    let () = Printf.printf "%s" hack in
    Exit_status.No_error
  | Error err ->
    let () =
      Printf.eprintf
        {| Received input that was not in the expected .ipynb format
(https://github.com/jupyter/nbformat/blob/main/nbformat/v4/nbformat.v4.schema.json)
Error:
%s"
|}
        err
    in
    Exit_status.Input_error

let hack_to_notebook () : Exit_status.t =
  let hack = Sys_utils.read_stdin_to_string () in
  match Hack_to_notebook.hack_to_notebook hack with
  | Ok ipynb_json ->
    let json_string =
      Hh_json.json_to_string ~sort_keys:true ~pretty:true ipynb_json
    in
    let () = print_endline json_string in
    Exit_status.No_error
  | Error err ->
    let () =
      Printf.eprintf
        {| Received input that was not in the expected format.
Expected a valid Hack file with special comments for notebook cell information.
Such files should only be created via `hh --notebook-to-hack`. Error:
%s
|}
        err
    in
    Exit_status.Input_error
