(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Utils
open String_utils

(**
 * Slash-escaped path in the system temp directory corresponding
 * with this root directory for this extension.
 *)
let path_of_root root extension =
  (* TODO: move this to places that write this file *)
  Sys_utils.mkdir_no_fail GlobalConfig.tmp_dir;
  let root_part = Path.slash_escaped_string_of_path root in
  Filename.concat GlobalConfig.tmp_dir (spf "%s.%s" root_part extension)

let is_of_root root fn =
  let root_part = Path.slash_escaped_string_of_path root in
  string_starts_with fn (Filename.concat GlobalConfig.tmp_dir root_part)

(**
 * Lock on this file will be held after the server has finished initializing.
 * *)
let lock_file root = path_of_root root "lock"

let log_link root = path_of_root root "log"

let pids_file root = path_of_root root "pids"

let socket_file root = path_of_root root "sock"

let dfind_log root = path_of_root root "dfind"

let client_lsp_log root = path_of_root root "client_lsp_log"

let client_ide_log root = path_of_root root "client_ide_log"

let monitor_log_link root = path_of_root root "monitor_log"

let server_finale_file (pid : int) : string =
  Filename.concat GlobalConfig.tmp_dir (spf "%d.fin" pid)

(* Return all the files that we need to typecheck *)
let make_next ~(indexer : unit -> string list) ~(extra_roots : Path.t list) :
    Relative_path.t list Bucket.next =
  let next_files_root =
    compose (List.map Relative_path.(create Root)) indexer
  in
  let hhi_root = Hhi.get_hhi_root () in
  let hhi_filter = FindUtils.is_hack in
  let next_files_hhi =
    compose
      (List.map Relative_path.(create Hhi))
      (Find.make_next_files ~name:"hhi" ~filter:hhi_filter hhi_root)
  in
  let rec concat_next_files l () =
    match l with
    | [] -> []
    | hd :: tl ->
      begin
        match hd () with
        | [] -> concat_next_files tl ()
        | x -> x
      end
  in
  let next_files_extra =
    List.map
      (fun root ->
        compose
          (List.map Relative_path.create_detect_prefix)
          (Find.make_next_files ~filter:FindUtils.file_filter root))
      extra_roots
    |> concat_next_files
  in
  fun () ->
    let next =
      concat_next_files [next_files_hhi; next_files_extra; next_files_root] ()
    in
    Bucket.of_list next
