(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let rec find_fbcode_dir (dir : string) : string option =
  let projectid_path = Filename.concat dir ".projectid" in
  if Sys.file_exists projectid_path then
    let content_opt =
      In_channel.with_file projectid_path ~f:In_channel.input_line
    in
    match content_opt with
    | Some content ->
      let content = String.strip content in
      if String.equal content "fbcode" then
        Some dir
      else
        find_fbcode_dir (Filename.dirname dir)
    | None -> find_fbcode_dir (Filename.dirname dir)
  else if String.equal dir "/" then
    None
  else
    find_fbcode_dir (Filename.dirname dir)

let set_working_directory_to_fbcode () : unit =
  let current_dir = Sys.getcwd () in
  match find_fbcode_dir current_dir with
  | Some fbcode_dir -> Sys.chdir fbcode_dir
  | None -> failwith "Searched up folders for fbcode but couldn't find it"

let is_facebook =
  let current_dir = Sys.getcwd () in
  let facebook_path = Filename.concat current_dir "facebook" in
  Sys.file_exists facebook_path && Sys.is_directory facebook_path

let () =
  let () = if is_facebook then set_working_directory_to_fbcode () in
  List.iter
    Generate_full_fidelity_data.templates
    ~f:Full_fidelity_schema.generate_file
