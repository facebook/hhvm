(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs
open Decl_defs
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
  let prefix = Prefix.make()
  let description = "Class"
end

(* a function type *)
module Fun = struct
  type t = decl Typing_defs.fun_type
  let prefix = Prefix.make()
  let description = "Fun"
end

module Typedef = struct
  type t = Typing_defs.typedef_type
  let prefix = Prefix.make()
  let description = "Typedef"
end

module GConst = struct
  type t = decl ty
  let prefix = Prefix.make()
  let description = "GConst"
end

module Funs = SharedMem.WithCache (StringKey) (Fun)
module Classes = SharedMem.WithCache (StringKey) (Class)
module Typedefs = SharedMem.WithCache (StringKey) (Typedef)
module GConsts = SharedMem.WithCache (StringKey) (GConst)

module Property = struct
  type t = decl ty
  let prefix = Prefix.make()
  let description = "Property"
end

module StaticProperty = struct
  type t = decl ty
  let prefix = Prefix.make()
  let description = "StaticProperty"
end

module Method = struct
  type t = decl fun_type
  let prefix = Prefix.make()
  let description = "Method"
end

module StaticMethod = struct
  type t = decl fun_type
  let prefix = Prefix.make()
  let description = "StaticMethod"
end

module Constructor = struct
  type t = decl fun_type
  let prefix = Prefix.make()
  let description = "Constructor"
end

module ClassEltKey = struct
  type t = string * string
  let compare (cls1, elt1) (cls2, elt2) =
    let r = String.compare cls1 cls2 in
    if r <> 0 then r
    else String.compare elt1 elt2

  let to_string (cls, elt) = cls ^ "::" ^ elt
end

module Props = SharedMem.WithCache (ClassEltKey) (Property)
module StaticProps = SharedMem.WithCache (ClassEltKey) (StaticProperty)
module Methods = SharedMem.WithCache (ClassEltKey) (Method)
module StaticMethods = SharedMem.WithCache (ClassEltKey) (StaticMethod)
module Constructors = SharedMem.WithCache (StringKey) (Constructor)
