(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val add_position_to_results :
  Provider_context.t -> Search_types.si_item list -> Search_utils.result

val autocomplete_result_to_json :
  Autocomplete_types.autocomplete_item -> Yojson.Safe.t

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  autocomplete_context:Autocomplete_types.legacy_autocomplete_context ->
  sienv_ref:Search_utils.si_env ref ->
  naming_table:Naming_table.t ->
  Autocomplete_types.autocomplete_item list Utils.With_complete_flag.t
