(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type local_tracking_env = {
  lte_fileinfos: FileInfo.t Relative_path.Map.t;
  lte_filenames: FileInfo.names Relative_path.Map.t;
}

(* Fetch current environment *)
val get_env: unit -> local_tracking_env

(* Size of local tracking environment *)
val count_local_fileinfos: local_tracking_env -> int

(* Update the tracked information for a file *)
val update_file: Relative_path.t -> SearchUtils.info -> unit

(* Clear out tracked information for a file *)
val remove_file: Relative_path.t -> unit

(* Search through locally tracked symbols *)
val search_local_symbols:
  query_text:string ->
  max_results:int ->
  kind_filter:SearchUtils.si_kind option ->
  env:local_tracking_env ->
  SearchUtils.si_results
