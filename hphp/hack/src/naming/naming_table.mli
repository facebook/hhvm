(**
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

type t
type fast = FileInfo.names Relative_path.Map.t
type saved_state_info = FileInfo.saved Relative_path.Map.t

(* Querying and updating forward naming tables. *)
val combine : t -> t -> t
val create : FileInfo.t Relative_path.Map.t -> t
val empty : t
val filter : t -> f:(Relative_path.t -> FileInfo.t -> bool) -> t
val fold : t -> init:'b -> f:(Relative_path.t -> FileInfo.t -> 'b -> 'b) -> 'b
val get_files : t -> Relative_path.t list
val get_file_info : t -> Relative_path.t -> FileInfo.t option
val get_file_info_unsafe : t -> Relative_path.t -> FileInfo.t
val has_file : t -> Relative_path.t -> bool
val iter : t -> f:(Relative_path.t -> FileInfo.t -> unit) -> unit
val update : t -> Relative_path.t -> FileInfo.t  -> t

(* Converting between different types of forward naming tables. *)
val from_saved : saved_state_info -> t
val to_saved : t -> saved_state_info
val to_fast: t -> fast
val saved_to_fast: saved_state_info -> fast

(* Querying and updating reverse naming tables. *)
module type ReverseNamingTable = sig
  type pos

  val add : string -> pos -> unit
  val get_pos : ?bypass_cache:bool -> string -> pos option
  val remove_batch : SSet.t -> unit

  val heap_string_of_key : string -> string
end

type type_of_type =
  | TClass
  | TTypedef
module Types : sig
  include ReverseNamingTable with type pos = FileInfo.pos * type_of_type
  val get_canon_name : string -> string option
end

module Funs : sig
  include ReverseNamingTable with type pos = FileInfo.pos
  val get_canon_name : string -> string option
end

module Consts : ReverseNamingTable with type pos = FileInfo.pos

val push_local_changes : unit -> unit
val pop_local_changes : unit -> unit
val has_local_changes : unit -> bool
