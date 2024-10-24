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

module ReadOnlyEnvironment : sig
  type t

  val default : verbose:int -> debug_pattern:string option -> t
end

module Environment : sig
  type t

  val default : t

  val definitions : t -> Definition.t list
end

module Type : sig
  type t

  val show : t -> string

  val inhabitant_of : ReadOnlyEnvironment.t -> Environment.t -> t -> string

  val mk : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t
end
