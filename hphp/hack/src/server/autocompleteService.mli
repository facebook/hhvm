(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val add_position_to_results :
  Provider_context.t -> SearchUtils.si_results -> SearchUtils.result

val autocomplete_result_to_json :
  AutocompleteTypes.autocomplete_item -> Hh_json.json

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  autocomplete_context:AutocompleteTypes.legacy_autocomplete_context ->
  sienv:SearchUtils.si_env ->
  AutocompleteTypes.autocomplete_item list Utils.With_complete_flag.t
