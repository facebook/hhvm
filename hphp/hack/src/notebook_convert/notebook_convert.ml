(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let notebook_to_hack ~(notebook_name : string) ~(header : string) :
    Exit_status.t =
  let source = Sys_utils.read_stdin_to_string () in
  let ipynb_json = Hh_json.json_of_string source in
  match Notebook_to_hack.notebook_to_hack ~notebook_name ~header ipynb_json with
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

let string_of_syntax_errors (syntax_errors : Full_fidelity_syntax_error.t list)
    : string =
  syntax_errors
  |> List.map ~f:Full_fidelity_syntax_error.show
  |> String.concat ~sep:", "

(* Assumes we only try to convert syntactically valid Hack into ipynb_json *)
let validate_hack_to_notebook_exn ipynb_json : unit =
  match
    Notebook_to_hack.notebook_to_hack
      ~notebook_name:"N12345"
      ~header:"// the_header"
      ipynb_json
  with
  | Ok hack2 ->
    let syntax_errors = snd @@ Notebook_convert_util.parse hack2 in
    if not @@ List.is_empty syntax_errors then
      let excerpt = String.sub hack2 ~pos:0 ~len:200 in
      failwith
      @@ Printf.sprintf
           {|Internal error: converting to hack notebook produced ipynb that can no longer be converted back to syntactically valid hack.
Excerpt:\n%s\nErrors: %s"|}
           excerpt
           (string_of_syntax_errors syntax_errors)
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
  let (tree, syntax_errors) = Notebook_convert_util.parse hack in
  if not @@ List.is_empty syntax_errors then
    let () =
      Printf.eprintf
        "Received syntactically invalid Hack: %s"
        (string_of_syntax_errors syntax_errors)
    in
    Exit_status.Input_error
  else
    match Hack_to_notebook.hack_to_notebook tree with
    | Ok ipynb_json ->
      let () = validate_hack_to_notebook_exn ipynb_json in
      let ipynb_json_string =
        Hh_json.json_to_string ~sort_keys:true ~pretty:true ipynb_json
      in
      let () = print_endline ipynb_json_string in
      Exit_status.No_error
    | Error (Notebook_convert_error.Internal err) -> raise err
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
