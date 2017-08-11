(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module EditableSyntax = Full_fidelity_editable_syntax
module MinimalSyntax = Full_fidelity_minimal_syntax
module SyntaxTree = Full_fidelity_syntax_tree
open FfpAutocompleteContextParser
open String_utils

let local_variable_valid_in_context (context:context) (stub:string) : bool =
  let open ContextPredicates in
  is_expression_valid context &&
  (stub = "" || string_starts_with stub "$")

let autocomplete_local
  ~(context:context)
  ~(file_content:string)
  ~(stub:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t)
  : AutocompleteTypes.complete_autocomplete_result list =
  if local_variable_valid_in_context context stub then
    let ac_results = ServerAutoComplete.auto_complete_at_position ~file_content ~pos ~tcopt in
    let open Utils.With_complete_flag in
    ac_results.value
  else
    []
