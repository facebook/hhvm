(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

val elaborate_program : (unit, unit) Aast.program -> Nast.program

val elaborate_fun_def : (unit, unit) Aast.fun_def -> Nast.fun_def

val elaborate_class_ : (unit, unit) Aast.class_ -> Nast.class_

val elaborate_module_def : (unit, unit) Aast.module_def -> Nast.module_def

val elaborate_gconst : (unit, unit) Aast.gconst -> Nast.gconst

val elaborate_typedef : (unit, unit) Aast.typedef -> Nast.typedef

val elaborate_stmt :
  ParserOptions.t -> ((unit, unit) Aast.stmt -> Nast.stmt) Staged.t
