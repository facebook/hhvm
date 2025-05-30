(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Keep in sync with //hphp/hack/src/facebook/utils/repo_root.rs *)

(**
 * Checks if x is a www directory by looking for ".hhconfig".
 *)
let is_www_directory ?(config = ".hhconfig") (path : Path.t) : bool =
  let arcconfig = Path.concat path config in
  Path.file_exists arcconfig

let assert_www_directory ?(config = ".hhconfig") (path : Path.t) : unit =
  if not (Path.file_exists path && Path.is_directory path) then (
    Printf.eprintf "Error: %s is not a directory\n%!" (Path.to_string path);
    exit 1
  );
  if not (is_www_directory ~config path) then (
    Printf.fprintf
      stderr
      "Error: could not find a %s file in %s or any of its parent directories. Do you have a %s in your code's root directory?\n%s\n"
      config
      (Path.to_string path)
      config
      (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
    flush stderr;
    exit 1
  )

(** Traverse parent directories until we find a directory containing .hhconfig *)
let rec guess_root config start ~recursion_limit : Path.t option =
  if start = Path.parent start then
    (* Reached file system root *)
    None
  else if is_www_directory ~config start then
    Some start
  else if recursion_limit <= 0 then
    None
  else
    guess_root config (Path.parent start) ~recursion_limit:(recursion_limit - 1)

let interpret_command_line_root_parameter
    ?(config = ".hhconfig") (paths : string list) : Path.t =
  let path =
    match paths with
    | [] -> "."
    | [path] -> path
    | _ ->
      Printf.fprintf
        stderr
        "Error: please provide at most one www directory\n%!";
      exit 1
  in
  let start_path = Path.make path in
  let root =
    match guess_root config start_path ~recursion_limit:50 with
    | None -> start_path
    | Some r -> r
  in
  assert_www_directory ~config root;
  root
