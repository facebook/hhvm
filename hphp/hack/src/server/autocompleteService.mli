(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val add_position_to_results :
  Provider_context.t -> SearchTypes.si_item list -> SearchUtils.result

val autocomplete_result_to_json :
  AutocompleteTypes.autocomplete_item -> Hh_json.json

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  autocomplete_context:AutocompleteTypes.legacy_autocomplete_context ->
  sienv_ref:SearchUtils.si_env ref ->
  naming_table:Naming_table.t ->
  AutocompleteTypes.autocomplete_item list Utils.With_complete_flag.t

val get_snippet_for_xhp_classname :
  Decl_provider.type_key -> Provider_context.t -> Tast_env.env -> string option
