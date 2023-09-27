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
  SearchUtils.si_env

(** Constructs+returns a mockable symbol-index *)
val mock :
  on_find:
    (query_text:string ->
    context:SearchTypes.autocomplete_type ->
    kind_filter:SearchTypes.si_kind option ->
    SearchTypes.si_item list) ->
  SearchUtils.si_env

(** This is the proper search function everyone should use.
The caller is expected to have done any namespace aliasing already,
e.g. if .hhconfig has auto_namespace_map={"Vec":"FlibSL\\Vec"}
then this function will give answers for query_text="FLibSL\Vec\ch" but not "Vec\ch". *)
val find_matching_symbols :
  sienv_ref:SearchUtils.si_env ref ->
  query_text:string ->
  max_results:int ->
  context:SearchTypes.autocomplete_type ->
  kind_filter:SearchTypes.si_kind option ->
  SearchTypes.si_item list * SearchTypes.si_complete

(** Does an approximate search for find-all-refs candidates.
If it returns None, then this functionality isn't available from this provider,
or isn't working for some reason.
If it returns Some, then these are best-guess candidates to check. *)
val find_refs :
  sienv_ref:SearchUtils.si_env ref ->
  action:SearchTypes.Find_refs.action ->
  max_results:int ->
  Relative_path.t list option
