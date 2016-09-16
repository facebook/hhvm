(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open File_content
open ServerEnv

let open_file env path =
  let path = Relative_path.(concat Root path) in
  let content =
    try Sys_utils.cat (Relative_path.to_absolute path) with _ -> "" in
  let fc = of_content ~content in
  let edited_files = Relative_path.Map.add env.edited_files path fc in
  let ide_needs_parsing =
    Relative_path.Set.add env.ide_needs_parsing path in
  let last_command_time = Unix.gettimeofday () in
  { env with
    edited_files; ide_needs_parsing; last_command_time;
  }

let close_file env path =
  let path = Relative_path.(concat Root path) in
  let edited_files = Relative_path.Map.remove env.edited_files path in
  let ide_needs_parsing =
    Relative_path.Set.remove env.ide_needs_parsing path in
  let last_command_time = Unix.gettimeofday () in
  { env with
    edited_files; ide_needs_parsing; last_command_time
  }

let edit_file env path edits =
  let path = Relative_path.(concat Root path) in
  let fc = try Relative_path.Map.find_unsafe env.edited_files path
  with Not_found ->
    let content =
      try Sys_utils.cat (Relative_path.to_absolute path) with _ -> "" in
    of_content ~content in
  let edited_fc = edit_file fc edits in
  let edited_files =
    Relative_path.Map.add env.edited_files path edited_fc in
  let ide_needs_parsing =
    Relative_path.Set.add env.ide_needs_parsing path in
  let last_command_time = Unix.gettimeofday () in
  { env with
    edited_files; ide_needs_parsing; last_command_time
  }
