(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Used by the Pessimised_shared_memory backend to decl-pessimise definitions
  not already found in the decl heap on-the-fly. *)
type pessimisation_info = {
  pessimise_shallow_class:
    Relative_path.t ->
    name:string ->
    Shallow_decl_defs.shallow_class ->
    Shallow_decl_defs.shallow_class;
  pessimise_fun:
    Relative_path.t -> name:string -> Typing_defs.fun_elt -> Typing_defs.fun_elt;
  pessimise_gconst:
    Relative_path.t ->
    name:string ->
    Typing_defs.const_decl ->
    Typing_defs.const_decl;
  pessimise_typedef:
    Relative_path.t ->
    name:string ->
    Typing_defs.typedef_type ->
    Typing_defs.typedef_type;
  allow_ast_caching: bool;
      (** The [AST_provider] must not use any caches (neither for ASTs or
        files) if this is unset *)
  store_pessimised_result: bool;
      (** Indicates whether or not decl-based pessimiation results (as
        determined by the functions above) should immediately be stored
        in the corresponding heaps. *)
}

module Cache (Entry : Lfu_cache.Entry) : module type of Lfu_cache.Cache (Entry)

module Decl_cache_entry : sig
  type _ t =
    | Fun_decl : string -> Typing_defs.fun_elt t
    | Class_decl : string -> Typing_class_types.class_t t
    | Typedef_decl : string -> Typing_defs.typedef_type t
    | Gconst_decl : string -> Typing_defs.const_decl t
    | Module_decl : string -> Typing_defs.module_def_type t

  type 'a key = 'a t

  type 'a value = 'a

  val compare : 'a key -> 'b key -> int

  val hash : 'a key -> int

  val key_to_log_string : 'a key -> string
end

module Decl_cache : module type of Cache (Decl_cache_entry)

module Shallow_decl_cache_entry : sig
  type _ t = Shallow_class_decl : string -> Shallow_decl_defs.shallow_class t

  type 'a key = 'a t

  type 'a value = 'a

  val compare : 'a key -> 'b key -> int

  val hash : 'a key -> int

  val key_to_log_string : 'a key -> string
end

module Shallow_decl_cache : module type of Cache (Shallow_decl_cache_entry)

module Folded_class_cache_entry : sig
  type _ t = Folded_class_decl : string -> Decl_defs.decl_class_type t

  type 'a key = 'a t

  type 'a value = 'a

  val compare : 'a key -> 'b key -> int

  val hash : 'a key -> int

  val key_to_log_string : 'a key -> string
end

module Folded_class_cache : module type of Cache (Folded_class_cache_entry)

(** A `fixme_map` associates:
    line number guarded by HH_FIXME =>
    error_node_number =>
    position of HH_FIXME comment *)
type fixme_map = Pos.t IMap.t IMap.t [@@deriving show]

module Fixme_store : sig
  (** a mutable store *)
  type t

  val empty : unit -> t

  val get : t -> Relative_path.t -> fixme_map option

  val add : t -> Relative_path.t -> fixme_map -> unit

  val remove : t -> Relative_path.t -> unit

  val remove_batch : t -> Relative_path.Set.t -> unit
end

module Fixmes : sig
  (** The `hh_fixmes` and `decl_hh_fixmes` fields represent the HH_FIXMEs we
      detected in each file in a full-parse and a decl-parse, respectively. The
      former will be a superset of the latter, when both are populated and
      up-to-date. We have both because in some scenarios, we will have only
      performed a decl-parse on a given file, but wouldn't want to take the
      FIXMEs we collected during a decl-parse (i.e., one in which we throw away
      function bodies and any FIXMEs they might contain) during a future
      full-typecheck of that file. When performing a lookup (in
      `Fixme_provider`), we check `hh_fixmes` first, and use `decl_hh_fixmes`
      only if there is no entry in `hh_fixmes`. *)
  type t = private {
    hh_fixmes: Fixme_store.t;
    decl_hh_fixmes: Fixme_store.t;
    disallowed_fixmes: Fixme_store.t;
  }

  val get_telemetry : key:string -> t -> Telemetry.t -> Telemetry.t
end

module Reverse_naming_table_delta : sig
  type pos = FileInfo.name_type * Relative_path.t

  type pos_or_deleted =
    | Pos of pos * pos list
        (** Pos(first,rest) is a multiset "first::rest" of positions.
        An arbitrary one of the positions is stored as 'first', and the
        rest are stored in 'rest'. This structure represents in ocaml's
        type system that the 'Pos' case has at least one element in its
        mltiset. Also, the first position is the one returned
        when a caller asks what is "the" position for a given symbol --
        many callers aren't even aware that there may be multiple positions,
        and will happily do something reasonable when given an arbitrary one.
        Our current implementation happens to leave 'first' unchanged until
        such time as it's removed, at which point it's arbitrary which of
        'rest' (if there are any) will be promoted to 'first'. *)
    | Deleted

  (** This stores a multimap from symbol name to the position(s)
  where it's defined. It also stores a lower-case version of the multimap. *)
  type t = {
    consts: pos_or_deleted SMap.t ref;
    funs: pos_or_deleted SMap.t ref;
    types: pos_or_deleted SMap.t ref;
    modules: pos_or_deleted SMap.t ref;
    funs_canon_key: pos_or_deleted SMap.t ref;
    types_canon_key: pos_or_deleted SMap.t ref;
  }

  val get_telemetry : key:string -> t -> Telemetry.t -> Telemetry.t
end

type local_memory = {
  shallow_decl_cache: Shallow_decl_cache.t;
      (** A cache for shallow classes.
        This corresponds to Shallow_class_heap.Classes used when we use the shared memory backend. *)
  folded_class_cache: Folded_class_cache.t;
      (** A cache for folded classes.
        This corresponds to Decl_heap.Classes used when we use the shared memory backend. *)
  decl_cache: Decl_cache.t;
      (** This contains top-level decls: functions, classish types, type aliases, global constants, etc.
        The classes in this cache correspond to Decl_provider.Cache used when we use the shared memory backend.
        The other top-level definitions correspond to Decl_heap.Typedefs/GConsts/Modules/etc. *)
  decls_reflect_this_file:
    (Relative_path.t * FileInfo.t * FileInfo.pfh_hash) option ref;
      (** Invariant: all decls in local_memory reflect the content of files as they
      are on disk, with the sole exception of [decls_reflect_this_file]: if this
      is Some, then decls reflect this file's string content rather than its disk
      content. (This is a special-case optimization, since the IDE typically typically
      handles an LSP request where it needs decls to reflect unsaved content of the
      active editor buffer, and then the very next LSP request is usually for the
      same editor buffer with same or similar content). *)
  reverse_naming_table_delta: Reverse_naming_table_delta.t;
      (** A map from symbol-name to pos. (1) It's used as a slowly updated
          authoritative place to look for symbols that have changed on disk since
          the naming-table sqlite. "Slow" means we might be asked to compute
          TASTs even before reverse_naming_table_delta has been updated
          to reflect all the change files on disk.
          It stores 'Deleted' for symbols which have been deleted since the
          saved-state; once a symbol is in the delta, it never leaves.
          (2) It's used as a cache of naming-table-sqlite lookups, to speed
          them up on subsequent queries, since sqlite is slow.
          (3) If a symbol is defined in two files, the delta will only point
          to an arbitrary one of those files.
          (4) It stores "FileInfo.pos" positions. These can be either filename-only
          or filename-line-col positions. There's no particular invariant enforced
          about this. We happen to store filename-only for file changes.
          (5) It stores names, and also canon_key (lowercase) names. For authoritative
          names, it always stores both. For cached names, it might store one or both.
          If two symbols have the same canon_key, then the canon_key points to
          an arbitrary one of them. *)
  fixmes: Fixmes.t;
  naming_db_path_ref: Naming_sqlite.db_path option ref;
}

type t =
  | Shared_memory  (** Used by hh_server and hh_single_type_check *)
  | Pessimised_shared_memory of pessimisation_info
  | Local_memory of local_memory  (** Used by serverless IDE *)
  | Rust_provider_backend of Rust_provider_backend.t
      (** For the Rust port of Provider_backend and decl-folding logic (rupro/hackrs) *)
  | Analysis

val t_to_string : t -> string

val set_analysis_backend : unit -> unit

val set_shared_memory_backend : unit -> unit

val set_pessimised_shared_memory_backend : pessimisation_info -> unit

val set_rust_backend : Rust_provider_backend.t -> unit

val set_local_memory_backend_with_defaults_for_test : unit -> unit

(** TODO(ljw): for now, max_num_shallow_decls accepts a special value "-1"
which reflects the status quo ante, a max size of 140mb in bytes rather than
a max number. This will be removed shortly. *)
val set_local_memory_backend :
  max_num_decls:int -> max_num_shallow_decls:int -> unit

val get_local_memory_telemetry : local_memory -> Telemetry.t

val get : unit -> t

val supports_eviction : t -> bool

val get_pessimised_shared_memory_backend_info : t -> pessimisation_info option

val is_pessimised_shared_memory_backend : t -> bool

val noop_pessimisation_info : pessimisation_info
