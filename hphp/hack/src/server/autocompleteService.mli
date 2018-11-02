(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val autocomplete_result_to_json :
  AutocompleteTypes.complete_autocomplete_result ->
  Hh_json.json

val go :
  tcopt:TypecheckerOptions.t ->
  delimit_on_namespaces:bool ->
  content_funs:Reordered_argument_collections.SSet.t ->
  content_classes:Reordered_argument_collections.SSet.t ->
  autocomplete_context: AutocompleteTypes.legacy_autocomplete_context ->
  basic_only:bool ->
  Tast.program ->
  AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t
