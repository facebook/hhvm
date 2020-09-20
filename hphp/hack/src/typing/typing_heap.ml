(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type ReadOnly = sig
  type key

  type t

  val get : key -> t option

  val mem : key -> bool

  val find_unsafe : key -> t
end

module Funs = Decl_heap.Funs
module Typedefs = Decl_heap.Typedefs
module GConsts = Decl_heap.GConsts
module RecordDefs = Decl_heap.RecordDefs
module Classes = Typing_classes_heap.Classes
