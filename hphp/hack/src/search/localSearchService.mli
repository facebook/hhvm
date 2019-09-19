(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Size of local tracking environment *)
val count_local_fileinfos : sienv:SearchUtils.si_env -> int

(* Returns an updated env adding the tracked information for a file *)
val update_file :
  sienv:SearchUtils.si_env ->
  path:Relative_path.t ->
  info:SearchUtils.info ->
  SearchUtils.si_env

(* Returns an updated env clearing out tracked information for a file *)
val remove_file :
  sienv:SearchUtils.si_env -> path:Relative_path.t -> SearchUtils.si_env

(* Search through locally tracked symbols *)
val search_local_symbols :
  sienv:SearchUtils.si_env ->
  query_text:string ->
  max_results:int ->
  context:SearchUtils.autocomplete_type option ->
  kind_filter:SearchUtils.si_kind option ->
  SearchUtils.si_results

(* Filter out anything that's been removed locally *)
val extract_dead_results :
  sienv:SearchUtils.si_env ->
  results:SearchUtils.si_results ->
  SearchUtils.si_results
