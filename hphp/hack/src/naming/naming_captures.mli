(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val populate_fun_def : Nast.fun_def -> Nast.fun_def

val populate_class_ : Nast.class_ -> Nast.class_

module Env : sig
  type t

  val empty : t
end

val elab_typedef :
  ?env:Env.t -> (unit, unit) Aast.typedef -> (unit, unit) Aast.typedef

val elab_fun_def :
  ?env:Env.t -> (unit, unit) Aast.fun_def -> (unit, unit) Aast.fun_def

val elab_module_def :
  ?env:Env.t -> (unit, unit) Aast.module_def -> (unit, unit) Aast.module_def

val elab_gconst :
  ?env:Env.t -> (unit, unit) Aast.gconst -> (unit, unit) Aast.gconst

val elab_class :
  ?env:Env.t -> (unit, unit) Aast.class_ -> (unit, unit) Aast.class_

val elab_program :
  ?env:Env.t -> (unit, unit) Aast.program -> (unit, unit) Aast.program
