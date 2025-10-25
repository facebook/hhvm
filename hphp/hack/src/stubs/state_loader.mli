(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type dirty_files = {
  master_changes: Relative_path.t list;
  local_changes: Relative_path.t list;
}

type hot_decls_paths = {
  legacy_hot_decls_path: string;
  shallow_hot_decls_path: string;
}

type native_load_result = {
  naming_table_path: string;
  corresponding_rev: Hg.rev;
  mergebase_rev: Hg.global_rev option;
  mergebase: Hg.Rev.t option Future.t;
  is_cached: bool;
  state_distance: int;
  deptable_fn: string;
  dirty_files: dirty_files Future.t;
  hot_decls_paths: hot_decls_paths;
  errors_path: string;
}

type saved_state_handle = {
  saved_state_for_rev: Hg.rev;
  saved_state_everstore_handle: string;
  watchman_mergebase: MonitorUtils.watchman_mergebase option;
}

type error = unit

type verbose_error = {
  message: string;
  stack: Utils.callstack;
  auto_retry: bool;
  environment: string option;
}
[@@deriving show]

val error_string_verbose : error -> verbose_error

val cached_state :
  load_64bit:bool ->
  ?saved_state_handle:saved_state_handle ->
  config_hash:string ->
  rev:Hg.rev ->
  (Hg.rev * string * MonitorUtils.watchman_mergebase option) option

val fetch_saved_state :
  load_64bit:bool ->
  ?cache_limit:int ->
  config:State_loader_config.timeouts ->
  config_hash:string ->
  saved_state_handle ->
  (string, error) result Future.t

val mk_state_future :
  config:State_loader_config.timeouts ->
  load_64bit:bool ->
  ?saved_state_handle:saved_state_handle ->
  config_hash:string option ->
  ignore_hh_version:bool ->
  ignore_hhconfig:bool ->
  use_prechecked_files:bool ->
  Path.t ->
  (native_load_result, error) result Future.t
