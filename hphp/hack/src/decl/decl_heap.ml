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

module Funs =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (StringKey)
    (Fun)
    (Capacity)

module Classes = struct
  include
    SharedMem.HeapWithLocalCache
      (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (StringKey)
      (Class)
      (Capacity)

  let add class_name decl =
    if Provider_backend.(get () = Analysis) then
      let stack = Printexc.get_callstack 10 in
      let () =
        Format.eprintf
          "Decl_heap: adding %s\n%s\n%!"
          class_name
          (Printexc.raw_backtrace_to_string stack)
      in
      failwith "bad decl"
    else
      add class_name decl
end

module Typedefs =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (StringKey)
    (Typedef)
    (Capacity)
module GConsts =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (StringKey)
    (GConst)
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
    if not (Core_kernel.Int.equal r 0) then
      r
    else
      String.compare elt1 elt2

  let to_string (cls, elt) = cls ^ "::" ^ elt
end

module Props =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (ClassEltKey)
    (Property)
    (Capacity)
module StaticProps =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (ClassEltKey)
    (StaticProperty)
    (Capacity)
module Methods =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (ClassEltKey)
    (Method)
    (Capacity)
module StaticMethods =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (ClassEltKey)
    (StaticMethod)
    (Capacity)
module Constructors =
  SharedMem.HeapWithLocalCache
    (SharedMem.ProfiledBackend (SharedMem.NonEvictable)) (StringKey)
    (Constructor)
    (Capacity)
