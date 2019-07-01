(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Autocomplete needs type information for functions, local variables, and class members, so we
   must call into the typechecker to do these types of completions. Once the typed AST is available,
   the call to the previous autocomplete should be replaced with examining the typed AST. *)
open FfpAutocompleteContextParser
open ContextPredicates
open Container
open String_utils

let local_variable_valid_in_context (context:context) (stub:string) : bool =
  is_expression_valid context &&
  (stub = "" || string_starts_with stub "$")

let should_complete_function (context:context) : bool =
  is_expression_valid context

let is_complete_class_member context =
  context.closest_parent_container = AfterDoubleColon ||
  context.closest_parent_container = AfterRightArrow

let run
  ~(context:context)
  ~(file_content:string)
  ~(stub:string)
  ~(pos:File_content.position)
  ~(tcopt:TypecheckerOptions.t)
  ~(basic_only:bool)
  ~(sienv:SearchUtils.si_env)
  : AutocompleteTypes.complete_autocomplete_result list =
  if local_variable_valid_in_context context stub ||
     should_complete_function context ||
     is_complete_class_member context
  then
    let ac_results = ServerAutoComplete.auto_complete_at_position
      ~is_manually_invoked:false ~delimit_on_namespaces:true ~file_content ~pos ~tcopt
      ~basic_only ~sienv
    in
    let open Utils.With_complete_flag in
    ac_results.value
  else
    []
