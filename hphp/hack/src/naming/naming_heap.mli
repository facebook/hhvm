(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Naming_heap is an internal implementation detail of Naming_provider
used for the shared-mem backend. Its job is to (1) store deltas to
the reverse naming table in shared-mem, (2) cache in sharedmem the
results of lookups in sqlite table. *)
module type ReverseNamingTable = sig
  type pos

  module Position : SharedMem.Value with type t = pos

  module CanonName : SharedMem.Value with type t = string

  val add : string -> pos -> unit

  val get_pos : Naming_sqlite.db_path option -> string -> pos option

  val remove_batch : Naming_sqlite.db_path option -> string list -> unit

  val get_canon_name : Provider_context.t -> string -> string option

  val hash : string -> Typing_deps.Dep.t

  val canon_hash : string -> Typing_deps.Dep.t
end

module Types : sig
  module TypeCanonHeap : SharedMem.Heap with type key = Typing_deps.Dep.t

  module TypePosHeap : SharedMem.Heap with type key = Typing_deps.Dep.t

  include
    ReverseNamingTable with type pos = FileInfo.pos * Naming_types.kind_of_type
end

module Funs : sig
  module FunCanonHeap : SharedMem.Heap with type key = Typing_deps.Dep.t

  module FunPosHeap : SharedMem.Heap with type key = Typing_deps.Dep.t

  include ReverseNamingTable with type pos = FileInfo.pos
end

module Consts : sig
  module ConstPosHeap : SharedMem.Heap with type key = Typing_deps.Dep.t

  include ReverseNamingTable with type pos = FileInfo.pos
end

(** This function searches all three namespaces (types, funs, consts) to
find which one contains each Dep.t. The earlier functions in this module
only search one specified namespace. Note: this function doesn't
use the sharedmem cache of names - doesn't benefit from it, doesn't
write into it. *)
val get_filenames_by_hash :
  Naming_sqlite.db_path option -> Typing_deps.DepSet.t -> Relative_path.Set.t

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit
