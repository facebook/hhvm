(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * This module holds all the operations that can be performed on forward and
 * reverse naming tables. Forward naming tables map from filenames to the
 * symbols in that file, while the reverse naming table maps from symbols to the
 * file that they're defined in.
 *)

type t [@@deriving show]

type changes_since_baseline

type fast = FileInfo.names Relative_path.Map.t

type saved_state_info = FileInfo.saved Relative_path.Map.t

type save_result = {
  files_added: int;
  symbols_added: int;
}

(* Querying and updating forward naming tables. *)
val combine : t -> t -> t

val empty : t

(** [filter] is implemented using tombstones on SQLite-backed naming tables, so
  * if your naming table is backed by SQLite you should try to avoid removing
  * more than half the table by filtering (otherwise it would be best to just
  * make a new empty one and add elements to it). On non-SQLite backed tables
  * we remove entries, so it's no more or less efficient depending on how many
  * are removed. *)
val filter : t -> f:(Relative_path.t -> FileInfo.t -> bool) -> t

val fold : t -> init:'b -> f:(Relative_path.t -> FileInfo.t -> 'b -> 'b) -> 'b

val get_files : t -> Relative_path.t list

val get_files_changed_since_baseline :
  changes_since_baseline -> Relative_path.t list

val get_file_info : t -> Relative_path.t -> FileInfo.t option

val get_file_info_unsafe : t -> Relative_path.t -> FileInfo.t

val has_file : t -> Relative_path.t -> bool

val iter : t -> f:(Relative_path.t -> FileInfo.t -> unit) -> unit

val remove : t -> Relative_path.t -> t

val update : t -> Relative_path.t -> FileInfo.t -> t

val update_many : t -> FileInfo.t Relative_path.Map.t -> t

val save : t -> string -> save_result

(* Creation functions. *)
val create : FileInfo.t Relative_path.Map.t -> t

(* The common path for loading a save state from a SQLite database *)
val load_from_sqlite : string -> t

(* This function is intended for incremental naming table saved state
    creation. It does not update the reverse naming table, so it should not
    be used when loading the naming table in type checking scenarios. *)
val load_from_sqlite_for_batch_update : string -> t

(* This function is intended for applying naming changes relative
  to the source code version on which the naming table was first created.
  For the additional naming changes to be compatible with a SQLite-backed
  naming table originally created on source code version X, they must be
  snapshotted by loading a naming table originally created on X and,
  at a later time, calling `save_changes_since_baseline`.
  The scenario where this can be useful is thus:
  1) In process A
    - load a SQLite-backed naming table for source code version N
    - user makes changes, resulting in source code version N + 1
    - save changes since baseline as C1
  2) In process B
    - load the naming table for N + C1
    - do some work
  3) In process A
    - user makes changes, resulting in source code version N + 2
    - save changes since baseline as C2
  4) In process B
    - load the naming table for N + C2
    - do some work
  This avoids having to send the naming table version N to process B more than
  once. After B gets naming table N, it only needs the changes C1 and C2 to
  restore the state as process A sees it.
  This is more relevant if A and B are not on the same host, and the cost
  of sending the naming table is not negligible.
  *)
val load_from_sqlite_with_changes_since_baseline :
  changes_since_baseline -> string -> t

(* The path to the table's SQLite database mapping filenames to symbol names, if any *)
val get_forward_naming_fallback_path : t -> string option

(* The path to the table's SQLite database mapping symbol names to filenames, if any.
    Analogous to SharedMem.loaded_dep_table_filename *)
val get_reverse_naming_fallback_path : unit -> string option

(* Converting between different types of forward naming tables. *)
val from_saved : saved_state_info -> t

val to_saved : t -> saved_state_info

val to_fast : t -> fast

val saved_to_fast : saved_state_info -> fast

val save_changes_since_baseline : t -> destination_path:string -> unit

(* Querying and updating reverse naming tables. *)
module type ReverseNamingTable = sig
  type pos

  val add : string -> pos -> unit

  val get_pos : ?bypass_cache:bool -> string -> pos option

  val get_filename : string -> Relative_path.t option

  val is_defined : string -> bool

  val remove_batch : SSet.t -> unit

  val heap_string_of_key : string -> string
end

type type_of_type =
  | TClass
  | TTypedef
  | TRecordDef
[@@deriving eq, enum]

module Types : sig
  include ReverseNamingTable with type pos = FileInfo.pos * type_of_type

  val get_kind : string -> type_of_type option

  val get_filename_and_kind : string -> (Relative_path.t * type_of_type) option

  val get_canon_name : Provider_context.t -> string -> string option
end

module Funs : sig
  include ReverseNamingTable with type pos = FileInfo.pos

  val get_canon_name : Provider_context.t -> string -> string option
end

module Consts : ReverseNamingTable with type pos = FileInfo.pos

val to_canon_name_key : string -> string

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit

val has_local_changes : unit -> bool

(* Test functions, do not use. *)
val assert_is_backed : t -> bool -> unit
