(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Dep : sig
  (** A node in the dependency graph that must be rechecked when its dependencies change. *)
  type dependent

  (** A node in the dependency graph that, when changed, must recheck all of its dependents. *)
  type dependency

  (** A type of dependency.

  An ['a variant] can be either the dependent or the dependency. For example,
  a function body can itself depend on a [Fun] (when it must be rechecked if
  the other function changed).

  A [dependency variant] can only be a dependency. Other symbols can take on
  this kind of dependency on something, but this kind of thing can't take a
  dependency on other symbols. For example, an "extends" is not a symbol in
  the code, so [Extends] cannot take on dependencies on other things. *)
  type _ variant =
    | GConst : string -> 'a variant
        (** Represents a global constant depending on something, or something
        depending on a global constant. *)
    | GConstName : string -> 'a variant
        (** Like [GConst], but used only in conservative redecl. May not be
        necessary anymore. *)
    | Const : (string * string) -> dependency variant
        (** Represents either a class constant depending on something, or
        something depending on a class constant. *)
    | AllMembers : string -> dependency variant
        (** Represents a dependency on all members of a class. Particularly
        useful for switch exhaustiveness-checking. We establish a dependency
        on all members of an enum in that case. *)
    | Class : string -> 'a variant
        (** Represents either a class depending on something, or something
        depending on a class. *)
    | RecordDef : string -> 'a variant
        (** Represents either a record depending on something, or something
        depending on a record. *)
    | Fun : string -> 'a variant
        (** Represents either a global function depending on something, or
        something depending on a global function. *)
    | FunName : string -> 'a variant
        (** Like [Fun], but used only in conservative redecl. May not be
        necessary anymore. *)
    | Prop : (string * string) -> dependency variant
        (** Represents either a class's instance property depending on
        something, or something depending on that property. *)
    | SProp : (string * string) -> dependency variant
        (** Represents either a class's static property depending on
        something, or something depending on that property. *)
    | Method : (string * string) -> dependency variant
        (** Represents either a class's instance method depending on
        something, or something depending on that method. *)
    | SMethod : (string * string) -> dependency variant
        (** Represents either a class's static method depending on
        something, or something depending on that method. *)
    | Cstr : string -> dependency variant
        (** Represents something depending on a class constructor. *)
    | Extends : string -> dependency variant
        (** Represents a class depending on an another class via an
        inheritance-like mechanism (`extends`, `implements`, `use`, `require
        extends`, `require implements`, etc.) *)

  type t

  val make : 'a variant -> t

  val is_class : t -> bool

  val extends_of_class : t -> t

  val compare : t -> t -> int

  val extract_name : 'a variant -> string

  val to_debug_string : t -> string

  val of_debug_string : string -> t

  val variant_to_string : 'a variant -> string
end

module DepSet : sig
  include module type of
      Reordered_argument_collections.Reordered_argument_set (Set.Make (Dep))

  val pp : Format.formatter -> t -> unit
end

val trace : bool ref

val add_dependency_callback :
  string ->
  (Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit) ->
  unit

(* returns the previous value of the flag *)
val allow_dependency_table_reads : bool -> bool

val add_idep : Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit

val get_ideps_from_hash : Dep.t -> DepSet.t

val get_ideps : Dep.dependency Dep.variant -> DepSet.t

val get_files : DepSet.t -> Relative_path.Set.t

val update_file : Relative_path.t -> FileInfo.t -> unit

(* Add to accumulator all extend dependencies of source_class. Visited is used
 * to avoid processing nodes reachable in multiple ways more than once. In other
 * words: use DFS to find all nodes reachable by "extends" edges starting from
 * source class *)
val get_extend_deps :
  visited:DepSet.t ref -> source_class:Dep.t -> acc:DepSet.t -> DepSet.t

(* Grow input set by adding all its extend dependencies (including recursive) *)
val add_extend_deps : DepSet.t -> DepSet.t

(* Grow input set by adding all its typing dependencies (direct only) *)
val add_typing_deps : DepSet.t -> DepSet.t

(* add_extend_deps and add_typing_deps chained together *)
val add_all_deps : DepSet.t -> DepSet.t
