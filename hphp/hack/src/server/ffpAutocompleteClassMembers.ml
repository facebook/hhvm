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
  let open Container in
  context.closest_parent_container = AfterDoubleColon ||
  context.closest_parent_container = AfterRightArrow

let autocomplete_class_member
  ~(context:context)
  ~(file_content:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t)
  : AutocompleteTypes.complete_autocomplete_result list =
  if is_complete_class_member context then
    let ac_results = ServerAutoComplete.auto_complete_at_position ~file_content ~pos ~tcopt in
    let open Utils.With_complete_flag in
    ac_results.value
  else
    []
