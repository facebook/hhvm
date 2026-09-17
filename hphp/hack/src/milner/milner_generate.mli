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

  val for_enum_initializer : t -> t
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

  (** A separate nullary completion witness associated with the existing payload
      binding. This does not replace the supplied payload type. *)
  val mk_procedure_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * (string * string) list

  (** A separate callable completion witness associated with the existing
      payload binding. Does not replace the supplied payload type. *)
  val mk_callable_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * (string * string) list

  val mk_enum_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * (string * string) list

  val mk_identity_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * (string * string) list

  val hierarchy_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    t ->
    Environment.t * (string * string) list

  val mk_callable : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t

  (** Generate a nullary procedure that either returns void or throws. *)
  val mk_procedure : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t

  (** Expected completion mode for a type returned by [mk_procedure]. *)
  val procedure_throws : t -> bool
end
