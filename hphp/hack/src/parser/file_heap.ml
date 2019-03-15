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

type disk_type = Disk of string | Ide of string

exception File_heap_stale

module FileHeap = SharedMem.WithCache (SharedMem.ProfiledImmediate) (Relative_path.S) (struct
    type t = disk_type
    let prefix = Prefix.make()
    let description = "Disk"
  end)

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
  | _ -> assert false
