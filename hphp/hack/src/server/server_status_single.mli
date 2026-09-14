(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  MultiWorker.worker list option ->
  Server_command_types.file_input list ->
  Provider_context.t ->
  return_expanded_tast:bool ->
  error_filter:Tast_provider.ErrorFilter.t ->
  Diagnostics.t * Tast.program Tast_with_dynamic.t Relative_path.Map.t

val go_from_cached_diagnostics :
  Server_env.env ->
  Server_command_types.file_input list ->
  return_expanded_tast:bool ->
  preexisting_warnings:bool ->
  is_stale:bool ->
  uses_partial_typecheck:bool ->
  error_filter:Tast_provider.ErrorFilter.t ->
  (Diagnostics.t * Tast.program Tast_with_dynamic.t Relative_path.Map.t) option
