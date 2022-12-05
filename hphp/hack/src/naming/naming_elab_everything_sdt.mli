(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This elaboration pass will:
  - pessimise return types; and
  - add `SupportDyn` attributes to classes; and
  - add upper bounds to type parameters.

  It is intended for use when the `--everything-sdt` typechecker option is set
*)
module Env : sig
  type t

  val empty : t

  val create : ?in_is_as:bool -> ?in_enum_class:bool -> unit -> t
end

include Naming_phase_sigs.Elaboration with module Env := Env
