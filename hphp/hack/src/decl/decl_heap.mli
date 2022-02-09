(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Decl_defs

module Capacity : sig
  val capacity : int
end

module Class : SharedMem.Value with type t = decl_class_type

module Fun : SharedMem.Value with type t = fun_elt

module Typedef : SharedMem.Value with type t = typedef_type

module GConst : SharedMem.Value with type t = const_decl

module Property : SharedMem.Value with type t = decl_ty

module StaticProperty : SharedMem.Value with type t = decl_ty

module Method : SharedMem.Value with type t = fun_elt

module StaticMethod : SharedMem.Value with type t = fun_elt

module Constructor : SharedMem.Value with type t = fun_elt

module ClassEltKey : SharedMem.Key with type t = string * string

module Funs :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (StringKey)
        (Fun)
        (Capacity)

module Classes :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (StringKey)
        (Class)
        (Capacity)

module Typedefs :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (StringKey)
        (Typedef)
        (Capacity)

module GConsts :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (StringKey)
        (GConst)
        (Capacity)

module Props :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (ClassEltKey)
        (Property)
        (Capacity)

module StaticProps :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (ClassEltKey)
        (StaticProperty)
        (Capacity)

module Methods :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (ClassEltKey)
        (Method)
        (Capacity)

module StaticMethods :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (ClassEltKey)
        (StaticMethod)
        (Capacity)

module Constructors :
    module type of
      SharedMem.HeapWithLocalCache
        (SharedMem.ProfiledBackend (SharedMem.Evictable)) (StringKey)
        (Constructor)
        (Capacity)
