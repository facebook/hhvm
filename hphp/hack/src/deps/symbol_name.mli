(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [Symbol_name.S] is really a string but we're transitioning to make it into an opaque type
which only supports eq+ord. *)
module type S = sig
  type t [@@deriving show, eq, ord]

  val of_string : string -> t

  val to_string_TRANSITIONAL : t -> string

  val of_string_TRANSITIONAL : string -> t
end

(** [Symbol_name.I] is really a string but we're transitioning to make it into an opaque type
which only supports deriving ord, plus case-insensitive ieq, plus case-sensitive eq.
Case-insensitive eq is done by [iequal]; it deliberately has an unconventional name because
the need for case-insensitivity is so rare it should not be accidentally stumbled upon.
Case-sensitive eq is done by [canonical] and then eq upon canonical. *)
module type I = sig
  type t [@@deriving show, ord]

  (** [s] is the case-sensitive version *)
  type s

  (** [iequal] does a case-insensitive comparison *)
  val iequal : t -> t -> bool

  (** [canonical] is the correctly-cased ("canonical") version of this name *)
  val canonical : t -> s

  val of_string : string -> t

  val to_string_TRANSITIONAL : t -> string

  val of_string_TRANSITIONAL : string -> t
end

(** [Symbol_name.Set] is a set that's currently implemented as SSet but we're
transitioning to make it into a set of an opaque type. *)
module type Set = sig
  include Stdlib.Set.S

  val pp : Format.formatter -> t -> unit

  val show : t -> string

  val to_sset_TRANSITIONAL : t -> SSet.t

  val of_sset_TRANSITIONAL : SSet.t -> t
end

(*****************************************************************************)
(* Opaque distinct types for Fun, Type and Const keys                        *)
(*****************************************************************************)

module Fun : S

module Type : S

module Const : S

module FunSet : Set with type elt := Fun.t

module TypeSet : Set with type elt := Type.t

module ConstSet : Set with type elt := Const.t

(*****************************************************************************)
(* Opaque distinct types for case-insensitive Fun, Type and Const keys       *)
(*****************************************************************************)

module IFun : I with type s = Fun.t

module IType : I with type s = Type.t

module IConst : I with type s = Const.t
