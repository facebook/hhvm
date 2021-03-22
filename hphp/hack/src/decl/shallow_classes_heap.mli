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

module Class : sig
  type t = shallow_class

  val prefix : Prefix.t

  val description : string
end

module Classes :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Class)
        (Capacity)

module FilterCapacity : sig
  val capacity : int
end

module Filter : sig
  type t = BloomFilter.t

  val prefix : Prefix.t

  val description : string
end

module MemberFilters : sig
  include module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Filter)
        (FilterCapacity)

  (**
   * Computes a Bloom Filter of the name of the members in the shallow class
   * and stores it in shared memory, using the name of the resptive class as
   * the key *)
  val add : shallow_class -> unit
end
