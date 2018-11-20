(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
   and type t = Typing_classes_heap.t

module Typedefs : ReadOnly
  with type key = StringKey.t
   and type t = typedef_type

module GConsts : ReadOnly
  with type key = StringKey.t
   and type t = decl ty * Errors.t
