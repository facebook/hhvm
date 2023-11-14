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

(* Updates symbol index from [si_addendum] generated from a previous parse of the file. *)
val update_file_from_addenda :
  sienv:SearchUtils.si_env ->
  path:Relative_path.t ->
  addenda:FileInfo.si_addendum list ->
  SearchUtils.si_env

(* Returns an updated env clearing out tracked information for a file *)
val remove_file :
  sienv:SearchUtils.si_env -> path:Relative_path.t -> SearchUtils.si_env

(* Search through locally tracked symbols *)
val search_local_symbols :
  sienv:SearchUtils.si_env ->
  query_text:string ->
  max_results:int ->
  context:SearchTypes.autocomplete_type ->
  kind_filter:FileInfo.si_kind option ->
  SearchTypes.si_item list

(* Filter out anything that's been removed locally *)
val extract_dead_results :
  sienv:SearchUtils.si_env ->
  results:SearchTypes.si_item list ->
  SearchTypes.si_item list
