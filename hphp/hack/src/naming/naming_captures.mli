(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val populate_fun_def :
  Nast.fun_def -> custom_err_config:Custom_error_config.t -> Nast.fun_def

val populate_class_ :
  Nast.class_ -> custom_err_config:Custom_error_config.t -> Nast.class_

val elab_typedef :
  (unit, unit) Aast.typedef ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.typedef

val elab_stmt :
  (unit, unit) Aast.stmt ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.stmt

val elab_fun_def :
  (unit, unit) Aast.fun_def ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.fun_def

val elab_module_def :
  (unit, unit) Aast.module_def ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.module_def

val elab_gconst :
  (unit, unit) Aast.gconst ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.gconst

val elab_class :
  (unit, unit) Aast.class_ ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.class_

val elab_program :
  (unit, unit) Aast.program ->
  custom_err_config:Custom_error_config.t ->
  (unit, unit) Aast.program
