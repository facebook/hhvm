(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This elaboration pass will replace Hsoft(_)` hints, with either
  - the inner hint to which the constructor is applied; OR
  - `Hlike`, when the `--interpret-soft-types-as-like-types` typechecker option
    is enabled
*)
module Env : sig
  type t

  val empty : t

  val soft_as_like : t -> bool

  val set_soft_as_like : t -> soft_as_like:bool -> t
end

include Naming_phase_sigs.Elaboration with module Env := Env
