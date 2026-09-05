(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* Keep in sync with //hphp/hack/src/facebook/utils/repo_root.rs *)

(**
 * Checks if x is a www directory by looking for ".hhconfig".
 *)
let is_www_directory ?(config = ".hhconfig") (path : Path.t) : bool =
  let arcconfig = Path.concat path config in
  Path.file_exists arcconfig

let validate_www_directory ~config (path : Path.t) : (unit, string) result =
  if not (Path.file_exists path && Path.is_directory path) then
    Error (Printf.sprintf "%s is not a directory" (Path.to_string path))
  else if not (is_www_directory ~config path) then
    Error
      (Printf.sprintf
         "could not find a %s file in %s or any of its parent directories. Do you have a %s in your code's root directory?"
         config
         (Path.to_string path)
         config)
  else
    Ok ()

let assert_www_directory ?(config = ".hhconfig") (path : Path.t) : unit =
  match validate_www_directory ~config path with
  | Ok () -> ()
  | Error message ->
    Printf.eprintf "Error: %s\n%!" message;
    exit 1

(** Traverse parent directories until we find a directory containing .hhconfig *)
let rec guess_root config start ~recursion_limit : Path.t option =
  if not (Path.file_exists start) then
    None
  else if Path.equal start (Path.dirname start) then
    (* Reached file system root *)
    None
  else if is_www_directory ~config start then
    Some start
  else if recursion_limit <= 0 then
    None
  else
    guess_root config (Path.dirname start) ~recursion_limit:(recursion_limit - 1)

let interpret_command_line_root_parameter
    ?(config = ".hhconfig") (paths : string list) : (Path.t, string) result =
  let open Result.Let_syntax in
  let* path =
    match paths with
    | [] -> Ok "."
    | [path] -> Ok path
    | _ -> Error "please provide at most one www directory"
  in
  let start_path = Path.make path in
  let root =
    match guess_root config start_path ~recursion_limit:50 with
    | None -> start_path
    | Some root -> root
  in
  let* () = validate_www_directory ~config root in
  Ok root
