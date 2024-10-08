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
