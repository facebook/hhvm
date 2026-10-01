(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(**
 * Checks if x is a www directory by looking for ".hhconfig".
 *)
let is_www_directory (path : Path.t) : bool =
  let hhconfig = Path.concat path ".hhconfig" in
  Path.file_exists hhconfig

let validate_www_directory (path : Path.t) : (unit, string) result =
  if not (Path.file_exists path && Path.is_directory path) then
    Error (Printf.sprintf "%s is not a directory" (Path.to_string path))
  else if not (is_www_directory path) then
    Error
      (Printf.sprintf
         "could not find a .hhconfig file in %s or any of its parent directories. Do you have a .hhconfig in your code's root directory?"
         (Path.to_string path))
  else
    Ok ()

let assert_www_directory (path : Path.t) : unit =
  match validate_www_directory path with
  | Ok () -> ()
  | Error message ->
    Printf.eprintf "Error: %s\n%!" message;
    exit 1

let guess_root (start : Path.t) : Path.t option =
  Repo_root_ffi.guess_root (Path.to_string start) |> Option.map ~f:Path.make

let interpret_command_line_root_parameter (paths : string list) :
    (Path.t, string) result =
  let open Result.Let_syntax in
  let* path =
    match paths with
    | [] -> Ok "."
    | [path] -> Ok path
    | _ -> Error "please provide at most one www directory"
  in
  let start_path = Path.make path in
  let root =
    match guess_root start_path with
    | Some root -> root
    | None ->
      let www_child = Path.concat start_path "www" in
      if is_www_directory www_child then
        www_child
      else
        start_path
  in
  let* () = validate_www_directory root in
  Ok root
