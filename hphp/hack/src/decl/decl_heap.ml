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

(* All the following heaps will have a local cache of size 1000 *)

module Capacity = struct
  let capacity = 1000
end

(* The following classes are used to make sure we make no typing
 * mistake when interacting with the database. The database knows
 * how to associate a string to a string. We need to deserialize
 * the string and make sure the type is correct. By using these
 * modules, the places where there could be a typing mistake are
 * very well isolated.
 *)

(* Module used to represent serialized classes *)
module Class = struct
  type t = decl_class_type

  let description = "Decl_Class"
end

(* a function type *)
module Fun = struct
  type t = fun_elt

  let description = "Decl_Fun"
end

module Typedef = struct
  type t = Typing_defs.typedef_type

  let description = "Decl_Typedef"
end

module GConst = struct
  type t = const_decl

  let description = "Decl_GConst"
end

module Module = struct
  type t = Typing_defs.module_def_type

  let description = "Decl_Module"
end

module Funs =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (StringKey)
    (Fun)
    (Capacity)
module Classes =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (StringKey)
    (Class)
    (Capacity)
module Typedefs =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (StringKey)
    (Typedef)
    (Capacity)
module GConsts =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (StringKey)
    (GConst)
    (Capacity)
module Modules =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (StringKey)
    (Module)
    (Capacity)

module Property = struct
  type t = decl_ty

  let description = "Decl_Property"
end

module StaticProperty = struct
  type t = decl_ty

  let description = "Decl_StaticProperty"
end

module Method = struct
  type t = fun_elt

  let description = "Decl_Method"
end

module StaticMethod = struct
  type t = fun_elt

  let description = "Decl_StaticMethod"
end

module Constructor = struct
  type t = fun_elt

  let description = "Decl_Constructor"
end

module ClassEltKey = struct
  type t = string * string

  let compare (cls1, elt1) (cls2, elt2) =
    let r = String.compare cls1 cls2 in
    if not (Core.Int.equal r 0) then
      r
    else
      String.compare elt1 elt2

  let to_string (cls, elt) = cls ^ "::" ^ elt
end

module Props =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (ClassEltKey)
    (Property)
    (Capacity)
module StaticProps =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (ClassEltKey)
    (StaticProperty)
    (Capacity)
module Methods =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (ClassEltKey)
    (Method)
    (Capacity)
module StaticMethods =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (ClassEltKey)
    (StaticMethod)
    (Capacity)
module Constructors =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (StringKey)
    (Constructor)
    (Capacity)
