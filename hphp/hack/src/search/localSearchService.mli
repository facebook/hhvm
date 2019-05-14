(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Size of local tracking environment *)
val count_local_fileinfos: SearchUtils.local_tracking_env -> int

(* Returns an updated env adding the tracked information for a file *)
val update_file:
  Relative_path.t ->
  SearchUtils.info ->
  SearchUtils.local_tracking_env -> SearchUtils.local_tracking_env

(* Returns an updated env clearing out tracked information for a file *)
val remove_file:
  Relative_path.t ->
  SearchUtils.local_tracking_env -> SearchUtils.local_tracking_env

(* Search through locally tracked symbols *)
val search_local_symbols:
  query_text:string ->
  max_results:int ->
  kind_filter:SearchUtils.si_kind option ->
  env:SearchUtils.local_tracking_env ->
  SearchUtils.si_results
