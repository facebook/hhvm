(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type build_progress

external build :
  www:string -> custom_hhi_path:string -> output:string -> build_progress
  = "naming_table_builder_ffi_build"

external poll : build_progress -> int option = "naming_table_builder_ffi_poll"
