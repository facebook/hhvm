(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
    Shared memory heap containing the contents of files.
    Acts as a sort of caching facade which is filled on-demand
    as contents are needed - The "cache" is filled by loading from
    the file system if the file isn't opened in the IDE
    (otherwise uses the IDE contents).
    That is, the IDE version take precedence over file system's.
*)

type file_type = Disk of string | Ide of string

exception File_provider_stale

module FileHeap = SharedMem.WithCache (SharedMem.ProfiledImmediate) (Relative_path.S) (struct
    type t = file_type
    let prefix = Prefix.make()
    let description = "Disk"
  end)

let get fn =
  FileHeap.get fn

let get_unsafe fn =
  match get fn with
  | Some contents -> contents
  | None ->
    failwith ("File not found: " ^ (Relative_path.to_absolute fn))

let get_contents fn =
  match FileHeap.get fn with
  | Some Ide f -> Some f
  | Some Disk contents -> Some contents
  | None ->
      let contents =
      try Sys_utils.cat (Relative_path.to_absolute fn) with _ -> "" in
      FileHeap.add fn (Disk contents);
      Some contents

let get_ide_contents_unsafe fn =
  match FileHeap.get fn with
  | Some Ide f -> f
  | _ ->
    failwith ("IDE file not found: " ^ (Relative_path.to_absolute fn))

let provide_file fn contents =
  FileHeap.add fn contents

let provide_file_hint fn contents =
  FileHeap.write_around fn contents

let remove_batch paths =
  FileHeap.remove_batch paths

let local_changes_push_stack () =
  FileHeap.LocalChanges.push_stack ()

let local_changes_pop_stack () =
  FileHeap.LocalChanges.pop_stack ()

let local_changes_commit_batch paths =
  FileHeap.LocalChanges.commit_batch paths

let local_changes_revert_batch paths =
  FileHeap.LocalChanges.revert_batch paths
