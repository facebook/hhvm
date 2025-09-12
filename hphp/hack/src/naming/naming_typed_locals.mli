(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val elab_fun_def :
  (unit, unit) Aast.fun_def ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.fun_def

val elab_class :
  (unit, unit) Aast.class_ ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.class_

val elab_program :
  (unit, unit) Aast.program ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.program

val elab_stmt :
  (unit, unit) Aast.stmt ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.stmt
