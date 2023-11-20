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
    | RequireExtends : string -> dependency variant
    | Const : string * string -> dependency variant
        (** Represents something depending on a class constant. *)
    | Constructor : string -> dependency variant
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
    | GConstName : string -> 'a variant
        (** Like [GConst], but used only in conservative redecl. May not be
        necessary anymore. *)
    | Module : string -> 'a variant
        (** Represents a toplevel symbol being defined as a member of
        this module *)
    | Declares : 'a variant
        (** An edge `Method(c, m) -> Declares` means that class c declares m. *)

  val dependency_of_variant : 'a variant -> dependency variant

  type dep_kind =
    | KGConst
    | KFun
    | KType
    | KExtends
    | KRequireExtends
    | KConst
    | KConstructor
    | KProp
    | KSProp
    | KMethod
    | KSMethod
    | KAllMembers
    | KGConstName
    | KModule
    | KDeclares
  [@@deriving enum]

  val dep_kind_of_variant : 'a variant -> dep_kind

  module Member : sig
    type t [@@deriving show]

    val method_ : string -> t

    val smethod : string -> t

    val prop : string -> t

    val sprop : string -> t

    val constructor : t

    val const : string -> t

    val all : t
  end

  (** A 63bit hash *)
  type t

  val make : 'a variant -> t

  val make_member_dep_from_type_dep : t -> Member.t -> t

  val compare_variant : 'a variant -> 'a variant -> int

  (** A 64bit representation of the 63bit hash. *)
  val to_int64 : t -> int64

  val to_int : t -> int

  val is_class : t -> bool

  val extends_and_req_extends_of_class : t -> t * t

  val compare : t -> t -> int

  val extract_name : 'a variant -> string

  val extract_root_name : ?strip_namespace:bool -> 'a variant -> string

  val extract_member_name : 'a variant -> string option

  val to_decl_reference : 'a variant -> Decl_reference.t

  val to_debug_string : t -> string

  val of_debug_string : string -> t

  val to_hex_string : t -> string

  val of_hex_string : string -> t

  val variant_to_string : 'a variant -> string

  val pp_variant : Format.formatter -> 'a variant -> unit
end

module DepHashKey : sig
  type t = Dep.t

  val compare : t -> t -> int

  val to_string : t -> string
end

module DepSet : sig
  type t [@@deriving show]

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

  val of_list : elt list -> t
end

module DepMap : sig
  include WrappedMap_sig.S with type key = Dep.t

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val show : (Format.formatter -> 'a -> unit) -> 'a t -> string
end

module VisitedSet : sig
  type t

  val make : unit -> t
end

val deps_of_file_info : FileInfo.t -> Dep.t list

type dep_edge

type dep_edges

val worker_id : int option ref

val trace : bool ref

val add_dependency_callback :
  name:string ->
  (Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit) ->
  unit

(** Return the previous value of the flag *)
val allow_dependency_table_reads : Mode.t -> bool -> bool

val add_idep :
  Mode.t -> Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit

val replace : Mode.t -> unit

val dep_edges_make : unit -> dep_edges

(** Depending on [mode], either return discovered edges
  which are not already in the dep graph
  or write those edges to disk. *)
val flush_ideps_batch : Mode.t -> dep_edges

val merge_dep_edges : dep_edges -> dep_edges -> dep_edges

(** Register the provided dep edges in the dep table delta in [typing_deps.rs] *)
val register_discovered_dep_edges : dep_edges -> unit

val remove_declared_tags : Mode.t -> DepSet.t -> unit

(** Save discovered edges to a binary file.

  - If mode is [InMemoryMode], the dep table delta in [typing_deps.rs] is saved.
  - If mode is [SaveToDiskMode], an exception is raised.

  Setting [reset_state_after_saving] will empty the dep table delta in
  [typing_deps.rs]. *)
val save_discovered_edges :
  Mode.t -> dest:string -> reset_state_after_saving:bool -> int

(** Load discovered edges from a binary file.

  - If mode is [InMemoryMode], the binary file is assumed to contain 64-bit
    hashes and they will be added to the dep table delta in [typing_deps.rs].
    If we have an existing table attached, we will first filter out edges
    that are already present in the attached table.
  - If mode is [SaveToDiskMode], the file is assumed to contain 64-bit
    hashes and they will be added ot the current worker's on-disk
    dependency edge file. *)
val load_discovered_edges : Mode.t -> string -> int

val get_ideps_from_hash : Mode.t -> Dep.t -> DepSet.t

val get_ideps : Mode.t -> Dep.dependency Dep.variant -> DepSet.t

(** Add to accumulator all extend dependencies of source_class. Visited is used
  to avoid processing nodes reachable in multiple ways more than once. In other
  words: use DFS to find all nodes reachable by "extends" edges starting from
  source class *)
val get_extend_deps :
  mode:Mode.t ->
  visited:VisitedSet.t ->
  source_class:Dep.t ->
  acc:DepSet.t ->
  DepSet.t

(** Grow input set by adding all its extend dependencies (including recursive) *)
val add_extend_deps : Mode.t -> DepSet.t -> DepSet.t

(** Grow input set by adding all its typing dependencies (direct only) *)
val add_typing_deps : Mode.t -> DepSet.t -> DepSet.t

(** add_extend_deps and add_typing_deps chained together *)
val add_all_deps : Mode.t -> DepSet.t -> DepSet.t

(** The fanout of a member `m` in type `A` contains:
  - the members `m` in descendants of `A` down to the first members `m` which are declared.
  - the dependents of those members `m` in descendants,
    but excluding dependents of declared members.

  We also include `A::m` itself in the result.
  The computed fanout is added to the provided DepSet accumulator. *)
val get_member_fanout :
  Mode.t -> class_dep:Dep.t -> Dep.Member.t -> DepSet.t -> DepSet.t

module Telemetry : sig
  val depgraph_delta_num_edges : Mode.t -> int option
end

val dump_current_edge_buffer_in_memory_mode :
  ?deps_to_symbol_map:'a Dep.variant DepMap.t -> unit -> unit
