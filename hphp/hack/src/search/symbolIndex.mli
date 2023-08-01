(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Get or set the currently selected search provider *)
val initialize :
  gleanopt:GleanOptions.t ->
  namespace_map:(string * string) list ->
  provider_name:string ->
  quiet:bool ->
  savedstate_file_opt:string option ->
  workers:MultiWorker.worker list option ->
  SearchUtils.si_env

(** Constructs+returns a mockable symbol-index *)
val mock :
  on_find:
    (query_text:string ->
    context:SearchTypes.autocomplete_type ->
    kind_filter:SearchTypes.si_kind option ->
    SearchTypes.si_item list) ->
  SearchUtils.si_env

(** This is the proper search function everyone should use *)
val find_matching_symbols :
  sienv_ref:SearchUtils.si_env ref ->
  query_text:string ->
  max_results:int ->
  context:SearchTypes.autocomplete_type ->
  kind_filter:SearchTypes.si_kind option ->
  SearchTypes.si_item list * SearchTypes.si_complete
