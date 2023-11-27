(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val elab_fun_def : (unit, unit) Aast.fun_def -> (unit, unit) Aast.fun_def

val elab_class : (unit, unit) Aast.class_ -> (unit, unit) Aast.class_

val elab_program : (unit, unit) Aast.program -> (unit, unit) Aast.program

val elab_stmt : (unit, unit) Aast.stmt -> (unit, unit) Aast.stmt
