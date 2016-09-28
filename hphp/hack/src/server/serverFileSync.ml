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
open Option.Monad_infix
open ServerEnv

let try_relativize_path x =
  Option.try_with (fun () -> Relative_path.(create Root x))

let open_file env path =
  let new_env = try_relativize_path path >>= fun path ->
    let content =
      try Sys_utils.cat (Relative_path.to_absolute path) with _ -> "" in
    let fc = of_content ~content in
    let edited_files = Relative_path.Map.add env.edited_files path fc in
    let ide_needs_parsing =
      Relative_path.Set.add env.ide_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      edited_files; ide_needs_parsing; last_command_time;
    } in
  Option.value new_env ~default:env

let close_file env path =
  let new_env = try_relativize_path path >>= fun path ->
    let edited_files = Relative_path.Map.remove env.edited_files path in
    let ide_needs_parsing =
      Relative_path.Set.add env.ide_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      edited_files; ide_needs_parsing; last_command_time
    } in
  Option.value new_env ~default:env

let edit_file env path edits =
  let new_env = try_relativize_path path >>= fun path ->
    let fc = try Relative_path.Map.find_unsafe env.edited_files path
    with Not_found ->
      let content =
        try Sys_utils.cat (Relative_path.to_absolute path) with _ -> "" in
      of_content ~content in
    let edited_fc = match edit_file fc edits with
      | Result.Ok r -> r
      | Result.Error e ->
        Hh_logger.log "%s" e;
        (* TODO: do not crash, but surface this to the client somehow *)
        assert false
    in
    let edited_files =
      Relative_path.Map.add env.edited_files path edited_fc in
    let ide_needs_parsing =
      Relative_path.Set.add env.ide_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      edited_files; ide_needs_parsing; last_command_time
    } in
  Option.value new_env ~default:env
