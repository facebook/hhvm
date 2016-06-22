(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Mapping the canonical name (lower case form) to the actual name *)
module type CanonHeap =
  SharedMem.NoCache with type t = string
               and type key = string
               and module KeySet = Set.Make (StringKey)

module TypeCanonHeap : CanonHeap = SharedMem.NoCache (StringKey) (struct
  type t = string
  let prefix = Prefix.make()
  let description = "TypeCanon"
end)

module FunCanonHeap : CanonHeap = SharedMem.NoCache (StringKey) (struct
  type t = string
  let prefix = Prefix.make()
  let description = "FunCanon"
end)

(* TypeIdHeap records both class names and typedefs since they live in the
 * same namespace. That is, one cannot both define a class Foo and a typedef
 * Foo (or FOO or fOo, due to case insensitivity). *)
module TypeIdHeap = SharedMem.WithCache (StringKey) (struct
  type t = Pos.t * [`Class | `Typedef]
  let prefix = Prefix.make ()
  let description = "TypeId"
end)

module FunPosHeap = SharedMem.NoCache (StringKey) (struct
  type t = Pos.t
  let prefix = Prefix.make()
  let description = "FunPos"
end)

module ConstPosHeap = SharedMem.NoCache (StringKey) (struct
  type t = Pos.t
  let prefix = Prefix.make()
  let description = "ConstPos"
end)
