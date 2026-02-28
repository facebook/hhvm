(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [Shallow_classes_heap] provides a cache of shallow class declarations. *)

open Shallow_decl_defs

module Capacity : sig
  val capacity : int
end

module Class : SharedMem.Value with type t = shallow_class

module Classes :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (StringKey)
        (Class)
        (Capacity)
