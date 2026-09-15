(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Size of local tracking environment *)
val count_local_fileinfos : sienv:Search_utils.si_env -> int

(* Updates symbol index from [si_addendum] generated from a previous parse of the file. *)
val update_file_from_addenda :
  sienv:Search_utils.si_env ->
  path:Relative_path.t ->
  addenda:File_info.si_addendum list ->
  Search_utils.si_env

(* Returns an updated env clearing out tracked information for a file *)
val remove_file :
  sienv:Search_utils.si_env -> path:Relative_path.t -> Search_utils.si_env

(* Search through locally tracked symbols *)
val search_local_symbols :
  sienv:Search_utils.si_env ->
  query_text:string ->
  max_results:int ->
  context:Search_types.autocomplete_type ->
  kind_filter:File_info.si_kind option ->
  Search_types.si_item list

(* Filter out anything that's been removed locally *)
val extract_dead_results :
  sienv:Search_utils.si_env ->
  results:Search_types.si_item list ->
  Search_types.si_item list
