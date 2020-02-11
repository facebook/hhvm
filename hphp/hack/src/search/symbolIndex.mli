(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Get or set the currently selected search provider *)
val initialize :
  globalrev:int option ->
  gleanopt:GleanOptions.t ->
  namespace_map:(string * string) list ->
  provider_name:string ->
  quiet:bool ->
  ignore_hh_version:bool ->
  savedstate_file_opt:string option ->
  workers:MultiWorker.worker list option ->
  SearchUtils.si_env

(* Log diagnostics for usage of autocomplete and symbol search *)
val log_symbol_index_search :
  sienv:SearchUtils.si_env ->
  query_text:string ->
  max_results:int ->
  results:int ->
  kind_filter:SearchUtils.si_kind option ->
  start_time:float ->
  context:SearchUtils.autocomplete_type option ->
  caller:string ->
  unit

(* This is the proper search function everyone should use *)
val find_matching_symbols :
  sienv:SearchUtils.si_env ->
  query_text:string ->
  max_results:int ->
  context:SearchUtils.autocomplete_type option ->
  kind_filter:SearchUtils.si_kind option ->
  SearchUtils.si_results

(* SLOWER: Update from a FileInfo.t object.  May need to do extra work to parse
 * into a usable format. *)
val update_files :
  ctx:Provider_context.t ->
  sienv:SearchUtils.si_env ->
  paths:(Relative_path.t * SearchUtils.info * SearchUtils.file_source) list ->
  SearchUtils.si_env

(* FASTER: Update from facts directly *)
val update_from_facts :
  sienv:SearchUtils.si_env ->
  path:Relative_path.t ->
  facts:Facts.facts ->
  SearchUtils.si_env

(* Notify the search service that certain files have been removed locally *)
val remove_files :
  sienv:SearchUtils.si_env -> paths:Relative_path.Set.t -> SearchUtils.si_env

(* Identify the position of an item *)
val get_position_for_symbol :
  Provider_context.t ->
  string ->
  SearchUtils.si_kind ->
  (Relative_path.t * int * int) option

(* Take an item and produce a position, or none if it cannot be found *)
val get_pos_for_item_opt :
  Provider_context.t -> SearchUtils.si_item -> Pos.absolute option

(* Take an item and produce a position, or a fake one if it cannot be found *)
val get_pos_for_item : Provider_context.t -> SearchUtils.si_item -> Pos.absolute
