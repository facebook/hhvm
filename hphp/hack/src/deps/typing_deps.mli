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
    | Fun : string -> 'a variant
        (** Represents either a global function depending on something, or
        something depending on a global function. *)
    | Class : string -> 'a variant
        (** Represents either a class depending on something, or something
        depending on a class. *)
    | Extends : string -> dependency variant
        (** Represents another class depending on a class via an
        inheritance-like mechanism (`extends`, `implements`, `use`, `require
        extends`, `require implements`, etc.) *)
    | RecordDef : string -> 'a variant
        (** Represents either a record depending on something, or something
        depending on a record. *)
    | Const : string * string -> dependency variant
        (** Represents something depending on a class constant. *)
    | Cstr : string -> dependency variant
        (** Represents something depending on a class constructor. *)
    | Prop : string * string -> dependency variant
        (** Represents something depending on a class's instance property. *)
    | SProp : string * string -> dependency variant
        (** Represents something depending on a class's static property. *)
    | Method : string * string -> dependency variant
        (** Represents something depending on a class's instance method. *)
    | SMethod : string * string -> dependency variant
        (** Represents something depending on a class's static method. *)
    | AllMembers : string -> dependency variant
        (** Represents something depending on all members of a class.
        Particularly useful for switch exhaustiveness-checking. We establish
        a dependency on all members of an enum in that case. *)
    | FunName : string -> 'a variant
        (** Like [Fun], but used only in conservative redecl. May not be
        necessary anymore. *)
    | GConstName : string -> 'a variant
        (** Like [GConst], but used only in conservative redecl. May not be
        necessary anymore. *)

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
  type t

  type elt = Dep.t

  val make : unit -> t

  val singleton : elt -> t

  val add : t -> elt -> t

  val union : t -> t -> t

  val inter : t -> t -> t

  val diff : t -> t -> t

  val iter : t -> f:(elt -> unit) -> unit

  val fold : t -> init:'a -> f:(elt -> 'a -> 'a) -> 'a

  val mem : t -> elt -> bool

  val elements : t -> elt list

  val cardinal : t -> int

  val is_empty : t -> bool

  val pp : Format.formatter -> t -> unit
end

module VisitedSet : sig
  type t

  val make : unit -> t
end

module NamingHash : sig
  (** A naming hash is like a dependency hash, but bigger. For technical
  reasons, dependency hashes are <32 bits, and they have some collisions in
  practice. Naming table hashes must uniquely identify symbols, so they have
  to be as close to 64 bits as possible to avoid collisions.

  We store these large naming table hashes directly in the naming table
  SQLite database. Then, when we calculate fanout from dependencies, we can
  use range queries with [make_lower_bound] and [make_upper_bound] to recover
  the naming table symbol from the dependency hash for a given symbol
  (possibly overestimating and including extra symbols, which is acceptable
  for fanout calculation). *)
  type t

  (** Create a naming table hash for the given symbol. *)
  val make : 'a Dep.variant -> t

  (** Produce a value such that [make_lower_bound (Dep.make x) <= make x] for
  all [x]. This can be used to query the naming table for symbols when you
  only have a dependency hash. *)
  val make_lower_bound : Dep.t -> t

  (** Produce a value such that [make x <= make_upper_bound (Dep.make x)] for
  all [x]. This can be used to query the naming table for symbols when you
  only have a dependency hash. *)
  val make_upper_bound : Dep.t -> t

  (** Convert the naming table hash into a value which can be stored into the
  naming table SQLite database. *)
  val to_int64 : t -> int64
end

module Files : sig
  val get_files : DepSet.t -> Relative_path.Set.t

  val deps_of_file_info : FileInfo.t -> DepSet.t

  val update_file : Relative_path.t -> FileInfo.t -> unit
end

type dep_edge

type dep_edges

type mode =
  | SQLiteMode
  | CustomMode of string
  | SaveCustomMode of {
      graph: string option;
      new_edges_dir: string;
    }
[@@deriving show]

val get_mode : unit -> mode

val set_mode : mode -> unit

val worker_id : int option ref

val force_load_custom_dep_graph : unit -> (unit, string) result

val trace : bool ref

val add_dependency_callback :
  string ->
  (Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit) ->
  unit

(* returns the previous value of the flag *)
val allow_dependency_table_reads : bool -> bool

val add_idep : Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit

val add_idep_directly_to_graph : dependent:Dep.t -> dependency:Dep.t -> unit

val dep_edges_make : unit -> dep_edges

val flush_ideps_batch : unit -> dep_edges

val merge_dep_edges : dep_edges -> dep_edges -> dep_edges

val register_discovered_dep_edges : dep_edges -> unit

val get_ideps_from_hash : Dep.t -> DepSet.t

val get_ideps : Dep.dependency Dep.variant -> DepSet.t

(* Add to accumulator all extend dependencies of source_class. Visited is used
 * to avoid processing nodes reachable in multiple ways more than once. In other
 * words: use DFS to find all nodes reachable by "extends" edges starting from
 * source class *)
val get_extend_deps :
  visited:VisitedSet.t -> source_class:Dep.t -> acc:DepSet.t -> DepSet.t

(* Grow input set by adding all its extend dependencies (including recursive) *)
val add_extend_deps : DepSet.t -> DepSet.t

(* Grow input set by adding all its typing dependencies (direct only) *)
val add_typing_deps : DepSet.t -> DepSet.t

(* add_extend_deps and add_typing_deps chained together *)
val add_all_deps : DepSet.t -> DepSet.t

module ForTest : sig
  val compute_dep_hash : 'a Dep.variant -> int

  val combine_hashes : dep_hash:int64 -> naming_hash:int64 -> int64
end
