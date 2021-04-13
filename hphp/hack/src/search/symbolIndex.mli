(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Get or set the currently selected search provider *)
val initialize :
  globalrev:int option ->
  gleanopt:GleanOptions.t ->
  namespace_map:(string * string) list ->
  provider_name:string ->
  quiet:bool ->
  ignore_hh_version:bool ->
  savedstate_file_opt:string option ->
  workers:MultiWorker.worker list option ->
  SearchUtils.si_env

(* This is the proper search function everyone should use *)
val find_matching_symbols :
  sienv:SearchUtils.si_env ->
  query_text:string ->
  max_results:int ->
  context:SearchUtils.autocomplete_type option ->
  kind_filter:SearchUtils.si_kind option ->
  SearchUtils.si_results
