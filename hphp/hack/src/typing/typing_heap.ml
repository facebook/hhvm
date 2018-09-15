(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module type ReadOnly = sig
  type key
  type t

  val get: key -> t option
  val mem: key -> bool
  val find_unsafe: key -> t
end

module Funs = Decl_heap.Funs
module Typedefs = Decl_heap.Typedefs
module GConsts = Decl_heap.GConsts

module Classes = struct
  module Class = struct
    type t = Typing_defs.class_type
    let prefix = Prefix.make()
    let description = "ClassType"
    let use_sqlite_fallback () = false
  end

  module Cache = SharedMem.LocalCache (StringKey) (Class)

  type key = StringKey.t
  type t = Class.t

  let get key =
    match Cache.get key with
    | Some c -> Some c
    | None ->
      match Decl_heap.Classes.get key with
      | Some c ->
        let class_type = Decl_class.to_class_type c in
        Cache.add key class_type;
        Some class_type
      | None ->
        None

  let find_unsafe key =
    match get key with
    | None -> raise Caml.Not_found
    | Some x -> x

  let mem key =
    match get key with
    | None -> false
    | Some _ -> true
end
