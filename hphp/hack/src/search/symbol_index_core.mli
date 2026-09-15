(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Log diagnostics for usage of autocomplete and symbol search *)
val log_symbol_index_search :
  sienv:Search_utils.si_env ->
  query_text:string ->
  max_results:int ->
  results:int ->
  kind_filter:File_info.si_kind option ->
  start_time:float ->
  caller:string ->
  unit

type paths_with_addenda =
  (Relative_path.t * File_info.si_addendum list * Search_utils.file_source) list

(* FASTER: update from addenda directly *)
val update_from_addenda :
  sienv:Search_utils.si_env ->
  paths_with_addenda:paths_with_addenda ->
  Search_utils.si_env

(* Notify the search service that certain files have been removed locally *)
val remove_files :
  sienv:Search_utils.si_env -> paths:Relative_path.Set.t -> Search_utils.si_env

(* Identify the position of an item *)
val get_position_for_symbol :
  Provider_context.t ->
  string ->
  File_info.si_kind ->
  (Relative_path.t * File_content.Position.t) option

(* Take an item and produce a position, or none if it cannot be found *)
val get_pos_for_item_opt :
  Provider_context.t -> Search_types.si_item -> Pos.absolute option

(* Take an item and produce a position, or a fake one if it cannot be found *)
val get_pos_for_item :
  Provider_context.t -> Search_types.si_item -> Pos.absolute
