(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types

val parse_analysis_mode : string -> analysis_mode option

val parse_refactor_mode : string -> refactor_mode option

val mk :
  analysis_mode:Refactor_sd_types.analysis_mode ->
  refactor_mode:Refactor_sd_types.refactor_mode ->
  options
