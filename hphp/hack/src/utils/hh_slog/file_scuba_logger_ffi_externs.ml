(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type logger

type logger_guard

external from_config_file : string -> logger * logger_guard
  = "file_scuba_logger_ffi_from_config_file"

external make_env_term : unit -> logger * logger_guard
  = "file_scuba_logger_ffi_make_env_term"
