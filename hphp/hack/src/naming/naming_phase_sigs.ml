(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type Elaboration = sig
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
end

module type Validation = sig
  module Env : sig
    type t

    val empty : t
  end

  val validate_typedef :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.typedef ->
    Naming_phase_error.t

  val validate_fun_def :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.fun_def ->
    Naming_phase_error.t

  val validate_module_def :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.module_def ->
    Naming_phase_error.t

  val validate_gconst :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.gconst ->
    Naming_phase_error.t

  val validate_class :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.class_ ->
    Naming_phase_error.t

  val validate_program :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.program ->
    Naming_phase_error.t
end

module type Elabidation = sig
  module Env : sig
    type t

    val empty : t
  end

  val elab_typedef :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.typedef ->
    (unit, unit) Aast.typedef * Naming_phase_error.t

  val elab_fun_def :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.fun_def ->
    (unit, unit) Aast.fun_def * Naming_phase_error.t

  val elab_module_def :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.module_def ->
    (unit, unit) Aast.module_def * Naming_phase_error.t

  val elab_gconst :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.gconst ->
    (unit, unit) Aast.gconst * Naming_phase_error.t

  val elab_class :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.class_ ->
    (unit, unit) Aast.class_ * Naming_phase_error.t

  val elab_program :
    ?init:Naming_phase_error.t ->
    ?env:Env.t ->
    (unit, unit) Aast.program ->
    (unit, unit) Aast.program * Naming_phase_error.t
end

type ('env, 'elem) validation =
  ?init:Naming_phase_error.t -> ?env:'env -> 'elem -> Naming_phase_error.t

type ('env, 'elem) elaboration = ?env:'env -> 'elem -> 'elem

type ('env, 'elem) elabidation =
  ?init:Naming_phase_error.t ->
  ?env:'env ->
  'elem ->
  'elem * Naming_phase_error.t
