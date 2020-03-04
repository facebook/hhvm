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

  let prefix = Prefix.make ()

  let description = "Decl_Class"
end

(* a function type *)
module Fun = struct
  type t = fun_elt

  let prefix = Prefix.make ()

  let description = "Decl_Fun"
end

module RecordDef = struct
  type t = Typing_defs.record_def_type

  let prefix = Prefix.make ()

  let description = "Decl_RecordDef"
end

module Typedef = struct
  type t = Typing_defs.typedef_type

  let prefix = Prefix.make ()

  let description = "Decl_Typedef"
end

module GConst = struct
  type t = decl_ty * Errors.t

  let prefix = Prefix.make ()

  let description = "Decl_GConst"
end

module Funs =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Fun) (Capacity)
module Classes =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Class)
    (Capacity)
module RecordDefs =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (RecordDef)
    (Capacity)
module Typedefs =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Typedef)
    (Capacity)
module GConsts =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (GConst)
    (Capacity)

module Property = struct
  type t = decl_ty

  let prefix = Prefix.make ()

  let description = "Decl_Property"
end

module StaticProperty = struct
  type t = decl_ty

  let prefix = Prefix.make ()

  let description = "Decl_StaticProperty"
end

module Method = struct
  type t = fun_elt

  let prefix = Prefix.make ()

  let description = "Decl_Method"
end

module StaticMethod = struct
  type t = fun_elt

  let prefix = Prefix.make ()

  let description = "Decl_StaticMethod"
end

module Constructor = struct
  type t = fun_elt

  let prefix = Prefix.make ()

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
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey) (Property)
    (Capacity)
module StaticProps =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey)
    (StaticProperty)
    (Capacity)
module Methods =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey) (Method)
    (Capacity)
module StaticMethods =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey) (StaticMethod)
    (Capacity)
module Constructors =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Constructor)
    (Capacity)
