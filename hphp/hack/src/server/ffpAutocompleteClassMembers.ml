(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open FfpAutocompleteContextParser

let is_complete_class_member context =
  let open FfpAutocompleteContextParser.Container in
  context.closest_parent_container = AfterDoubleColon ||
  context.closest_parent_container = AfterRightArrow

let legacy_auto_complete
  ~(file_content:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t)
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t=
  let open Ide_api_types in
  (* TODO: Avoid doing the "AUTO332" thing by modifying autocomplete service *)
  (* TODO: Call the method for getting global fns/classes separately *)
  (* TODO: Only run the parser/typechecker hooks for class members *)
  let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
  let content = File_content.edit_file_unsafe file_content edits in
  ServerAutoComplete.auto_complete
    ~tcopt
    ~delimit_on_namespaces:true
    content

let autocomplete_class_member
  ~(context:context)
  ~(file_content:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t)
  : AutocompleteTypes.complete_autocomplete_result list =
  if is_complete_class_member context then
    let ac_results = legacy_auto_complete ~file_content ~pos ~tcopt in
    let open Utils.With_complete_flag in
    ac_results.value
  else
    []
