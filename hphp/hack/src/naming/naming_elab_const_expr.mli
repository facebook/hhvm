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

  val create : ?in_enum_class:bool -> ?in_mode:FileInfo.mode -> unit -> t
end

include Naming_phase_sigs.Elabidation with module Env := Env
