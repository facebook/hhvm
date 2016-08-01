(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

module type ReadOnly = sig
  type key
  type t

  val get: key -> t option
  val mem: key -> bool
  val find_unsafe: key -> t
end

module Funs : ReadOnly
  with type key = StringKey.t
   and type t = decl fun_type

module Classes : ReadOnly
  with type key = StringKey.t
   and type t = class_type

module Typedefs : ReadOnly
  with type key = StringKey.t
   and type t = typedef_type

module GConsts : ReadOnly
  with type key = StringKey.t
   and type t = decl ty
