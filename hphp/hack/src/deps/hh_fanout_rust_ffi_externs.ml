(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type hh_fanout_rust_ffi

(* Keep order of arguments consistent with Rust FFI! *)

external make :
  logger:File_scuba_logger_ffi_externs.logger ->
  fanout_state_dir:string ->
  decl_state_dir:string ->
  hh_fanout_rust_ffi = "hh_fanout_ffi_make"

external make_hhdg_builder :
  logger:File_scuba_logger_ffi_externs.logger ->
  builder_state_dir:string ->
  hh_fanout_rust_ffi = "hh_fanout_ffi_make_hhdg_builder"
