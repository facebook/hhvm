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

module Class : Shared_mem.Value with type t = decl_class_type

module Fun : Shared_mem.Value with type t = fun_elt

module Typedef : Shared_mem.Value with type t = typedef_type

module GConst : Shared_mem.Value with type t = const_decl

module Module : Shared_mem.Value with type t = module_def_type

module Property : Shared_mem.Value with type t = decl_ty

module StaticProperty : Shared_mem.Value with type t = decl_ty

module Method : Shared_mem.Value with type t = fun_elt

module StaticMethod : Shared_mem.Value with type t = fun_elt

module Constructor : Shared_mem.Value with type t = fun_elt

module ClassEltKey : Shared_mem.Key with type t = string * string

module Funs :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (String_key)
        (Fun)
        (Capacity)

module Classes :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (String_key)
        (Class)
        (Capacity)

module Typedefs :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (String_key)
        (Typedef)
        (Capacity)

module GConsts :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (String_key)
        (GConst)
        (Capacity)

module Modules :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (String_key)
        (Module)
        (Capacity)

module Props :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (ClassEltKey)
        (Property)
        (Capacity)

module StaticProps :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (ClassEltKey)
        (StaticProperty)
        (Capacity)

module Methods :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (ClassEltKey)
        (Method)
        (Capacity)

module StaticMethods :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (ClassEltKey)
        (StaticMethod)
        (Capacity)

module Constructors :
    module type of
      Shared_mem.HeapWithLocalCache
        (Shared_mem.ImmediateBackend (Shared_mem.Evictable)) (String_key)
        (Constructor)
        (Capacity)
