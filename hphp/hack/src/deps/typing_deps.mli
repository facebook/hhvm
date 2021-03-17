(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Mode = Typing_deps_mode

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
    | Type : string -> 'a variant
        (** Represents either a class/typedef/recorddef/trait/interface depending on something,
        or something depending on one. *)
    | Extends : string -> dependency variant
        (** Represents another class depending on a class via an
        inheritance-like mechanism (`extends`, `implements`, `use`, `require
        extends`, `require implements`, etc.) *)
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

  val make : Mode.hash_mode -> 'a variant -> t

  val is_class : t -> bool

  val extends_of_class : t -> t

  val compare : t -> t -> int

  val extract_name : 'a variant -> string

  val to_decl_reference : 'a variant -> Decl_reference.t

  val to_debug_string : t -> string

  val of_debug_string : string -> t

  val variant_to_string : 'a variant -> string
end

module DepSet : sig
  type t

  type elt = Dep.t

  val make : Mode.t -> t

  val singleton : Mode.t -> elt -> t

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

  val of_list : Mode.t -> elt list -> t

  val pp : Format.formatter -> t -> unit
end

module VisitedSet : sig
  type t

  val make : Mode.t -> t
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

  val deps_of_file_info : Mode.t -> FileInfo.t -> Dep.t list

  val update_file : Mode.t -> Relative_path.t -> FileInfo.t -> unit
end

type dep_edge

type dep_edges

val hash_mode : Typing_deps_mode.t -> Typing_deps_mode.hash_mode

val worker_id : int option ref

val trace : bool ref

val add_dependency_callback :
  string ->
  (Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit) ->
  unit

(* returns the previous value of the flag *)
val allow_dependency_table_reads : Mode.t -> bool -> bool

val add_idep :
  Mode.t -> Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit

val add_idep_directly_to_graph :
  Mode.t -> dependent:Dep.t -> dependency:Dep.t -> unit

val dep_edges_make : unit -> dep_edges

val flush_ideps_batch : Mode.t -> dep_edges

val merge_dep_edges : dep_edges -> dep_edges -> dep_edges

val register_discovered_dep_edges : dep_edges -> unit

(** Save discovered edges to a binary file.
  *
  * - If mode is [SQLiteMode], the full dep table in shared memory is saved.
  * - If mode is [CustomMode], the dep table delta in [typing.rs] is saved.
  * - If mode is [SaveCustomMode], an exception is raised.
  *
  * Setting [reset_state_after_saving] will empty either shared memory or the
  * dep table delta in [typing.rs], depending on the mode.
  *
  * Currently, [build_revision] is ignored.
  *)
val save_discovered_edges :
  Mode.t ->
  dest:string ->
  build_revision:string ->
  reset_state_after_saving:bool ->
  int

(** Load discovered edges from a binary file.
  *
  * - If mode is [SQLiteMode], the binary file is assumed to contain 32-bit
  *   hashes, and they will all be added to the shared memory table.
  * - If mode is [CustomMode], the binary file is assumed to contain 64-bit
  *   hashes and they will be added to the dep table delta in [typing.rs].
  *   If we have an existing table attached, we will first filter out edges
  *   that are already present in the attached table.
  * - If mode is [SaveCustomMode], an exception is raised.
  *
  * Currently, [ignore_hh_version] is ignored.
  *)
val load_discovered_edges : Mode.t -> string -> ignore_hh_version:bool -> int

val get_ideps_from_hash : Mode.t -> Dep.t -> DepSet.t

val get_ideps : Mode.t -> Dep.dependency Dep.variant -> DepSet.t

(* Add to accumulator all extend dependencies of source_class. Visited is used
 * to avoid processing nodes reachable in multiple ways more than once. In other
 * words: use DFS to find all nodes reachable by "extends" edges starting from
 * source class *)
val get_extend_deps :
  mode:Mode.t ->
  visited:VisitedSet.t ->
  source_class:Dep.t ->
  acc:DepSet.t ->
  DepSet.t

(* Grow input set by adding all its extend dependencies (including recursive) *)
val add_extend_deps : Mode.t -> DepSet.t -> DepSet.t

(* Grow input set by adding all its typing dependencies (direct only) *)
val add_typing_deps : Mode.t -> DepSet.t -> DepSet.t

(* add_extend_deps and add_typing_deps chained together *)
val add_all_deps : Mode.t -> DepSet.t -> DepSet.t

module ForTest : sig
  val compute_dep_hash : Mode.hash_mode -> 'a Dep.variant -> int

  val combine_hashes : dep_hash:int64 -> naming_hash:int64 -> int64
end
