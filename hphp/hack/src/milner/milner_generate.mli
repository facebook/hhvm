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

  (** Restrict the outer type to legal alias right-hand sides. *)
  val for_alias : t -> t
end

module Environment : sig
  type t

  val default : t

  val definitions : t -> Definition.t list
end

module Type : sig
  type t

  val show : t -> string

  (** Exclude the documented completeness bugs in intersection-law templates. *)
  val intersection_law_compatible :
    Environment.t -> t -> Environment.t -> t -> bool

  (** Generate a scoped expression and retain any declarations it introduces. *)
  val inhabitant_of :
    ReadOnlyEnvironment.t -> Environment.t -> t -> Environment.t * string

  val subtype_of : ReadOnlyEnvironment.t -> Environment.t -> t -> t

  val mk : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t
end
