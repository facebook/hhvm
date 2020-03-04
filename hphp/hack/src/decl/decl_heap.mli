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

module Class : sig
  type t = decl_class_type

  val prefix : Prefix.t

  val description : string
end

module Fun : sig
  type t = fun_elt

  val prefix : Prefix.t

  val description : string
end

module Typedef : sig
  type t = typedef_type

  val prefix : Prefix.t

  val description : string
end

module RecordDef : sig
  type t = record_def_type

  val prefix : Prefix.t

  val description : string
end

module GConst : sig
  type t = decl_ty * Errors.t

  val prefix : Prefix.t

  val description : string
end

module Property : sig
  type t = decl_ty

  val prefix : Prefix.t

  val description : string
end

module StaticProperty : sig
  type t = decl_ty

  val prefix : Prefix.t

  val description : string
end

module Method : sig
  type t = fun_elt

  val prefix : Prefix.t

  val description : string
end

module StaticMethod : sig
  type t = fun_elt

  val prefix : Prefix.t

  val description : string
end

module Constructor : sig
  type t = fun_elt

  val prefix : Prefix.t

  val description : string
end

module ClassEltKey : SharedMem.UserKeyType with type t = string * string

module Funs :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Fun)
        (Capacity)

module Classes :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Class)
        (Capacity)

module RecordDefs :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (RecordDef)
        (Capacity)

module Typedefs :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Typedef)
        (Capacity)

module GConsts :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (GConst)
        (Capacity)

module Props :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey) (Property)
        (Capacity)

module StaticProps :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey)
        (StaticProperty)
        (Capacity)

module Methods :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey) (Method)
        (Capacity)

module StaticMethods :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (ClassEltKey)
        (StaticMethod)
        (Capacity)

module Constructors :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey)
        (Constructor)
        (Capacity)
