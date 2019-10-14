(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val fuzzy_search_enabled : unit -> bool

val set_fuzzy_search_enabled : bool -> unit

(* Get or set the currently selected search provider *)
val initialize :
  globalrev:int option ->
  gleanopt:GleanOptions.t ->
  namespace_map:(string * string) list ->
  provider_name:string ->
  quiet:bool ->
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

(* Notify the search service that certain files have been updated locally *)
val update_files :
  sienv:SearchUtils.si_env ref ->
  workers:MultiWorker.worker list option ->
  paths:(Relative_path.t * SearchUtils.info * SearchUtils.file_source) list ->
  unit

(* Notify the search service that certain files have been removed locally *)
val remove_files :
  sienv:SearchUtils.si_env ref -> paths:Relative_path.Set.t -> unit

(* Identify the position of an item *)
val get_position_for_symbol :
  string -> SearchUtils.si_kind -> (Relative_path.t * int * int) option

(* Take an item and produce a position, or none if it cannot be found *)
val get_pos_for_item_opt : SearchUtils.si_item -> Pos.absolute option

(* Take an item and produce a position, or a fake one if it cannot be found *)
val get_pos_for_item : SearchUtils.si_item -> Pos.absolute
