(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


val fuzzy_search_enabled : unit -> bool
val set_fuzzy_search_enabled : bool -> unit

(* Write the name of the current search provider to the HH_Log *)
val log_search_provider: unit -> unit

(* This is the proper search function everyone should use *)
val symbol_index_query :
    string ->
    int ->
    SearchUtils.si_kind option ->
    SearchUtils.si_results

(* Legacy query interface that depends on multiworker *)
val query :
  MultiWorker.worker list option ->
  string ->
  string ->
  fuzzy:bool ->
  (Pos.t, SearchUtils.search_result_type) SearchUtils.term list

(* Legacy query interface that depends on filter-map *)
val query_for_autocomplete :
  string ->
  limit:int option ->
  filter_map:(
    string ->
    string ->
    (FileInfo.pos, SearchUtils.search_result_type) SearchUtils.term ->
    'a option) ->
  'a list Utils.With_complete_flag.t

(* Legacy query interface for class searching *)
val query_class_methods :
  string ->
  string ->
  (Pos.t, SearchUtils.search_result_type) SearchUtils.term list

(* Legacy update interface when new data is seen by the typechecker *)
val update :
  MultiWorker.worker list option ->
  (Relative_path.t * SearchUtils.info) list ->
  unit

  (* Legacy update interface when old data is being cleaned out by the typechecker *)
val remove_files :
  Relative_path.Set.t ->
  unit
