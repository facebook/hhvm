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

module RecordDef : SharedMem.Value with type t = record_def_type

module GConst : SharedMem.Value with type t = const_decl

module Property : SharedMem.Value with type t = decl_ty

module StaticProperty : SharedMem.Value with type t = decl_ty

module Method : SharedMem.Value with type t = fun_elt

module StaticMethod : SharedMem.Value with type t = fun_elt

module Constructor : SharedMem.Value with type t = fun_elt

module ClassEltKey : SharedMem.Key with type t = string * string

module Funs :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (StringKey) (Fun)
        (Capacity)

module Classes :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (StringKey) (Class)
        (Capacity)

module RecordDefs :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (StringKey) (RecordDef)
        (Capacity)

module Typedefs :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (StringKey) (Typedef)
        (Capacity)

module GConsts :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (StringKey) (GConst)
        (Capacity)

module Props :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (ClassEltKey) (Property)
        (Capacity)

module StaticProps :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (ClassEltKey)
        (StaticProperty)
        (Capacity)

module Methods :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (ClassEltKey) (Method)
        (Capacity)

module StaticMethods :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (ClassEltKey)
        (StaticMethod)
        (Capacity)

module Constructors :
    module type of
      SharedMem.WithCache (SharedMem.ProfiledBackend) (StringKey) (Constructor)
        (Capacity)
