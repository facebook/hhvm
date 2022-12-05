(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Env : sig
  type t = bool

  val empty : t
end

include Naming_phase_sigs.Elabidation with module Env := Env
