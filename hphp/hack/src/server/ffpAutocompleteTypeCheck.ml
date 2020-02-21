(*
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

let local_variable_valid_in_context (context : context) (stub : string) : bool =
  is_expression_valid context && (stub = "" || string_starts_with stub "$")

let should_complete_function (context : context) : bool =
  is_expression_valid context

let is_complete_class_member context =
  context.closest_parent_container = AfterDoubleColon
  || context.closest_parent_container = AfterRightArrow

let run
    ~(context : context)
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(stub : string)
    ~(pos : File_content.position)
    ~(sienv : SearchUtils.si_env) :
    AutocompleteTypes.complete_autocomplete_result list =
  if
    local_variable_valid_in_context context stub
    || should_complete_function context
    || is_complete_class_member context
  then
    let ac_results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerAutoComplete.go_ctx
            ~ctx
            ~entry
            ~sienv
            ~is_manually_invoked:false
            ~line:pos.File_content.line
            ~column:pos.File_content.column)
    in
    ac_results.AutocompleteTypes.completions
  else
    []
