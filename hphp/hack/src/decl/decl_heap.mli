(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Decl_defs

module Class : sig
  type t = decl_class_type
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module Fun : sig
  type t = decl fun_type
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module Typedef :
  sig
    type t = typedef_type
    val prefix : Prefix.t
    val description : string
    val use_sqlite_fallback : unit -> bool
  end
module GConst : sig
  type t = decl ty * Errors.t
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module Property : sig
  type t = decl ty
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module StaticProperty : sig
  type t = decl ty
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module Method : sig
  type t = decl fun_type
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module StaticMethod : sig
  type t = decl fun_type
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end
module Constructor : sig
  type t = decl fun_type
  val prefix : Prefix.t
  val description : string
  val use_sqlite_fallback : unit -> bool
end

module ClassEltKey : SharedMem.UserKeyType
  with type t = string * string

module Funs : module type of SharedMem.WithCache (StringKey) (Fun)
module Classes : module type of SharedMem.WithCache (StringKey) (Class)
module Typedefs : module type of SharedMem.WithCache (StringKey) (Typedef)
module GConsts : module type of SharedMem.WithCache (StringKey) (GConst)

module Props :
  module type of SharedMem.WithCache (ClassEltKey) (Property)
module StaticProps :
  module type of SharedMem.WithCache (ClassEltKey) (StaticProperty)
module Methods :
  module type of SharedMem.WithCache (ClassEltKey) (Method)
module StaticMethods :
  module type of SharedMem.WithCache (ClassEltKey) (StaticMethod)
module Constructors :
  module type of SharedMem.WithCache (StringKey) (Constructor)
