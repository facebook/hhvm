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
    the file system.
*)

exception File_provider_stale

module FileHeap = struct
  include
    SharedMem.Heap
      (SharedMem.ImmediateBackend (SharedMem.Evictable)) (Relative_path.S)
      (struct
        type t = string

        let description = "File"
      end)

  let replace_nonatomic key value =
    if mem key then remove key;
    add key value
end

let read_file_contents_from_disk (fn : Relative_path.t) : string option =
  try Some (Sys_utils.cat (Relative_path.to_absolute fn)) with
  | _ -> None

let get_contents ?(force_read_disk = false) fn =
  match Provider_backend.get () with
  | Provider_backend.Analysis
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    let from_cache =
      if force_read_disk then
        None
      else
        FileHeap.get fn
    in
    (match from_cache with
    | Some contents -> Some contents
    | None ->
      let contents =
        Option.value (read_file_contents_from_disk fn) ~default:""
      in
      Some contents)
  | Provider_backend.Rust_provider_backend backend ->
    Some (Rust_provider_backend.File.get_contents backend fn)
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    read_file_contents_from_disk fn

let provide_file_for_tests fn contents =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    FileHeap.replace_nonatomic fn contents
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.File.provide_file_for_tests backend fn contents
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      "File_provider.provide_file_for_tests not supported with local/decl memory provider"

let remove_batch paths =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    FileHeap.remove_batch paths
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.File.remove_batch backend paths
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    failwith
      "File_provider.remove_batch not supported with local/decl memory provider"

let local_changes_push_sharedmem_stack () = FileHeap.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () = FileHeap.LocalChanges.pop_stack ()
