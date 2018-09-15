(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(*****************************************************************************)
(** Utils for processing parsed client args. *)
(*****************************************************************************)

let rec guess_root config start recursion_limit : Path.t option =
  if start = Path.parent start then None (* Reach fs root, nothing to do. *)
  else if Wwwroot.is_www_directory ~config start then Some start
  else if recursion_limit <= 0 then None
  else guess_root config (Path.parent start) (recursion_limit - 1)


let get_root ?(config=".hhconfig") path_opt =
  let start_str = match path_opt with
    | None -> "."
    | Some s -> s in
  let start_path = Path.make start_str in
  let root = match guess_root config start_path 50 with
    | None -> start_path
    | Some r -> r in
  Wwwroot.assert_www_directory ~config root;
  root
