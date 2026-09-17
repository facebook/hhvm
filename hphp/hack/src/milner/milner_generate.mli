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

  val inhabitant_of : ReadOnlyEnvironment.t -> Environment.t -> t -> string

  val subtype_of : ReadOnlyEnvironment.t -> Environment.t -> t -> t

  val mk : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t

  (** Independent constructor and member operations for one generated hierarchy.
      The supplied type is the payload; all operations share its environment. *)
  type generic_witness = {
    generic_family: string;
    generic_key: t;
    generic_payload: t;
    generic_narrow: t;
    generic_class: t;
    generic_wide: t;
    generic_reader: t;
    generic_writer: t;
    generic_tagged_class: t;
    generic_tagged_writer: t;
  }

  val mk_generic_witness :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * generic_witness

  type dependent_witness = {
    dependent_class: string;
    dependent_base: string;
    dependent_payload: t;
    dependent_bound: t;
    dependent_item: t;
    dependent_read: string;
    dependent_read_bound: string;
  }

  val mk_dependent_witness :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * dependent_witness

  val hierarchy_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    t ->
    Environment.t * (string * string) list
end
