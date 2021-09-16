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

  val get_pos : Naming_sqlite.db_path option -> string -> pos option

  val get_filename :
    Naming_sqlite.db_path option -> string -> Relative_path.t option

  val is_defined : Naming_sqlite.db_path option -> string -> bool

  val remove_batch : Naming_sqlite.db_path option -> string list -> unit

  module Position : SharedMem.Value with type t = pos
end

module Types : sig
  module CanonName : SharedMem.Value with type t = string

  module TypeCanonHeap :
    SharedMem.NoCache
      with type key = string
       and module KeyHasher = SharedMem.MakeKeyHasher(StringKey)

  module TypePosHeap :
    SharedMem.WithCache
      with type key = string
       and module KeyHasher = SharedMem.MakeKeyHasher(StringKey)

  include
    ReverseNamingTable with type pos = FileInfo.pos * Naming_types.kind_of_type

  val get_kind :
    Naming_sqlite.db_path option -> string -> Naming_types.kind_of_type option

  val get_filename_and_kind :
    Naming_sqlite.db_path option ->
    string ->
    (Relative_path.t * Naming_types.kind_of_type) option

  val get_canon_name : Provider_context.t -> string -> string option
end

module Funs : sig
  module CanonName : SharedMem.Value with type t = string

  module FunCanonHeap :
    SharedMem.NoCache
      with type key = string
       and module KeyHasher = SharedMem.MakeKeyHasher(StringKey)

  module FunPosHeap :
    SharedMem.NoCache
      with type key = string
       and module KeyHasher = SharedMem.MakeKeyHasher(StringKey)

  include ReverseNamingTable with type pos = FileInfo.pos

  val get_canon_name : Provider_context.t -> string -> string option
end

module Consts : sig
  module ConstPosHeap :
    SharedMem.NoCache
      with type key = string
       and module KeyHasher = SharedMem.MakeKeyHasher(StringKey)

  include ReverseNamingTable with type pos = FileInfo.pos
end

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit

val has_local_changes : unit -> bool
