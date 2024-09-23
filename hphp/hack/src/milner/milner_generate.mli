(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Definition : sig
  type t

  val show : t -> string
end

module Environment : sig
  type t

  val default : t
end

module Type : sig
  type t

  val inhabitant_of : t -> string

  val show : t -> string

  val mk : Environment.t -> depth:int option -> Definition.t list * t
end
