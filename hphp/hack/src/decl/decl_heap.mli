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

module Class : Value.Type with type t = decl_class_type

module Fun : Value.Type with type t = fun_elt

module Typedef : Value.Type with type t = typedef_type

module RecordDef : Value.Type with type t = record_def_type

module GConst : Value.Type with type t = const_decl

module Property : Value.Type with type t = decl_ty

module StaticProperty : Value.Type with type t = decl_ty

module Method : Value.Type with type t = fun_elt

module StaticMethod : Value.Type with type t = fun_elt

module Constructor : Value.Type with type t = fun_elt

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
