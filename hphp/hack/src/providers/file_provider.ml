(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*
    Shared memory heap containing the contents of files.
    Acts as a sort of caching facade which is filled on-demand
    as contents are needed - The "cache" is filled by loading from
    the file system if the file isn't opened in the IDE
    (otherwise uses the IDE contents).
    That is, the IDE version take precedence over file system's.
*)

type file_type =
  | Disk of string
  | Ide of string

exception File_provider_stale

module FileHeap =
  SharedMem.NoCache (SharedMem.ProfiledImmediate) (Relative_path.S)
    (struct
      type t = file_type

      let prefix = Prefix.make ()

      let description = "File"
    end)

let read_file_contents_from_disk (fn : Relative_path.t) : string option =
  (try Some (Sys_utils.cat (Relative_path.to_absolute fn)) with _ -> None)

let get fn =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> FileHeap.get fn
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith "File_provider.get not supported with local/decl memory provider"

let get_unsafe fn =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    begin
      match get fn with
      | Some contents -> contents
      | None -> failwith ("File not found: " ^ Relative_path.to_absolute fn)
    end
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      "File_provider.get_unsafe not supported with local/decl memory provider"

let get_contents fn =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    begin
      match FileHeap.get fn with
      | Some (Ide f) -> Some f
      | Some (Disk contents) -> Some contents
      | None ->
        let contents =
          Option.value (read_file_contents_from_disk fn) ~default:""
        in
        FileHeap.add fn (Disk contents);
        Some contents
    end
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    read_file_contents_from_disk fn

let get_ide_contents_unsafe fn =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    begin
      match FileHeap.get fn with
      | Some (Ide f) -> f
      | _ -> failwith ("IDE file not found: " ^ Relative_path.to_absolute fn)
    end
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      ( "File_provider.get_ide_contents_unsafe not supported "
      ^ "with local/decl memory provider" )

let provide_file fn contents =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> FileHeap.add fn contents
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      "File_provider.provide_file not supported with local/decl memory provider"

let provide_file_hint fn contents =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> FileHeap.add fn contents
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      "File_provider.provide_file_hint not supported with local/decl memory provider"

let remove_batch paths =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> FileHeap.remove_batch paths
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      "File_provider.remove_batch not supported with local/decl memory provider"

let local_changes_push_sharedmem_stack () = FileHeap.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () = FileHeap.LocalChanges.pop_stack ()

let local_changes_commit_batch paths =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> FileHeap.LocalChanges.commit_batch paths
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      ( "File_provider.local_changes_commit_batch not supported "
      ^ "with local/decl memory provider" )

let local_changes_revert_batch paths =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> FileHeap.LocalChanges.revert_batch paths
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      ( "File_provider.local_changes_revert_batch not supported "
      ^ "with local/decl memory provider" )
