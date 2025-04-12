(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let notebook_to_hack ~(notebook_number : string) ~(header : string) :
    Exit_status.t =
  let source = Sys_utils.read_stdin_to_string () in
  (* workaround for hh_json not dealing with unicode characters encoded as \u....*)
  let hack =
    match Yojson.Safe.from_string source with
    | exception Yojson.Json_error e -> Error e
    | ipynb_yojson ->
      let ipynb_json = Hh_json.of_yojson ipynb_yojson in
      Notebook_to_hack.notebook_to_hack ~notebook_number ~header ipynb_json
  in
  match hack with
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

(* Assumes we only try to convert syntactically valid Hack into ipynb_json *)
let validate_hack_to_notebook_exn ipynb_json : unit =
  match
    Notebook_to_hack.notebook_to_hack
      ~notebook_number:"N12345"
      ~header:"// the_header"
      ipynb_json
  with
  | Ok _ -> ()
  | Error err ->
    failwith
    @@ Printf.sprintf
         "Internal error: converting to notebook produced ipynb json that can't be converted back into hack. Error: %s\nipynb_json:\n%s"
         err
         (Hh_json.json_to_string ~sort_keys:true ~pretty:true ipynb_json)

(**
* Note: We're careful to distinguish user errors from internal errors
* because .php files may be edited in userland.
* (unlike .ipynb JSON, which is typically not human-edited)
*)
let hack_to_notebook () : Exit_status.t =
  let hack = Sys_utils.read_stdin_to_string () in
  match Hack_to_notebook.hack_to_notebook hack with
  | Ok ipynb_json ->
    let () = validate_hack_to_notebook_exn ipynb_json in
    let ipynb_json_string =
      Hh_json.json_to_string ~sort_keys:true ~pretty:true ipynb_json
    in
    let () = print_endline ipynb_json_string in
    Exit_status.No_error
  | Error (Notebook_convert_error.Invalid_input msg) ->
    let () =
      Printf.eprintf
        {| Received input that was not in the expected format.
Expected a valid Hack file with special comments for notebook cell information.
Such files should only be created via `hh --notebook-to-hack`. Error:
%s
|}
        msg
    in
    Exit_status.Input_error
