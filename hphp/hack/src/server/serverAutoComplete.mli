(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  sienv_ref:SearchUtils.si_env ref ->
  naming_table:Naming_table.t ->
  is_manually_invoked:bool ->
  line:int ->
  column:int ->
  AutocompleteTypes.ide_result

val go_at_auto332_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  sienv_ref:SearchUtils.si_env ref ->
  autocomplete_context:AutocompleteTypes.legacy_autocomplete_context ->
  naming_table:Naming_table.t ->
  AutocompleteTypes.autocomplete_item list Utils.With_complete_flag.t

val get_autocomplete_context :
  file_content:string ->
  pos:File_content.position ->
  is_manually_invoked:bool ->
  AutocompleteTypes.legacy_autocomplete_context

val get_signature : Provider_context.t -> string -> string option
