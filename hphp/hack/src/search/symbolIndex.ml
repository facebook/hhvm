(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

include HackSearchService

let fuzzy_search_enabled () = !fuzzy
let set_fuzzy_search_enabled x = fuzzy := x

let query = MasterApi.query
let query_for_autocomplete = MasterApi.query_autocomplete
let query_class_methods = ClassMethods.query

type info =
  | Full of FileInfo.t
  | Fast of FileInfo.names

let index_symbols_in_file (fn, info) =
  match info with
  | Full infos -> WorkerApi.update_from_fileinfo fn infos
  | Fast names -> WorkerApi.update_from_fast fn names

let update workers files =
  if List.length files < 100
  then List.iter files index_symbols_in_file
  else
    MultiWorker.call workers
      ~job:(fun () files -> List.iter files index_symbols_in_file)
      ~neutral:()
      ~merge:(fun _ _ -> ())
      ~next:(MultiWorker.next workers files);

  MasterApi.update_search_index ~fuzzy:!fuzzy (List.map files fst)

let remove_files = MasterApi.clear_shared_memory
