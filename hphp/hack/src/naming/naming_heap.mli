(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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

module Types : sig
  include
    ReverseNamingTable with type pos = FileInfo.pos * Naming_types.kind_of_type

  val get_kind : string -> Naming_types.kind_of_type option

  val get_filename_and_kind :
    string -> (Relative_path.t * Naming_types.kind_of_type) option

  val get_canon_name : Provider_context.t -> string -> string option
end

module Funs : sig
  include ReverseNamingTable with type pos = FileInfo.pos

  val get_canon_name : Provider_context.t -> string -> string option
end

module Consts : ReverseNamingTable with type pos = FileInfo.pos

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit

val has_local_changes : unit -> bool
