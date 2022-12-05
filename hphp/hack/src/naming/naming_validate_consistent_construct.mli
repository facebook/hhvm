(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Env : sig
  type t

  val empty : t

  val consistent_ctor_level : t -> int

  val set_consistent_ctor_level : t -> consistent_ctor_level:int -> t
end

include Naming_phase_sigs.Validation with module Env := Env
