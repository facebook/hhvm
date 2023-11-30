(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_deps
open Decl_heap

module VersionedNames : sig
  type t = {
    old_names: FileInfo.names;
    new_names: FileInfo.names;
  }
  [@@deriving show]

  val empty : t

  val make_unchanged : FileInfo.names -> t

  val merge : t -> t -> t
end

module VersionedSSet : sig
  type t = {
    old: SSet.t;
    new_: SSet.t;
  }

  type diff = {
    removed: SSet.t;
    kept: SSet.t;
    added: SSet.t;
  }

  val get_classes : VersionedNames.t -> t

  val empty : t

  val merge : t -> t -> t

  val diff : t -> diff
end

module VersionedFileInfo : sig
  module Diff : sig
    type t = {
      funs: VersionedSSet.diff;
      types: VersionedSSet.diff;
      gconsts: VersionedSSet.diff;
      modules: VersionedSSet.diff;
    }
  end

  val diff_names : VersionedNames.t -> Diff.t
end

val class_big_diff : Class.t -> Class.t -> bool

module ClassDiff : sig
  val compare : Class.t -> Class.t -> bool
end

module ClassEltDiff : sig
  val compare : Class.t -> Class.t -> [> `Changed | `Unchanged ]
end

val get_extend_deps : Typing_deps_mode.t -> DepSet.elt -> DepSet.t -> DepSet.t

val get_funs_deps :
  ctx:Provider_context.t ->
  Funs.value option SMap.t ->
  VersionedSSet.diff ->
  Fanout.t * int

val get_types_deps :
  ctx:Provider_context.t ->
  Typedef.t option SMap.t ->
  VersionedSSet.diff ->
  Fanout.t * int

val get_gconsts_deps :
  ctx:Provider_context.t ->
  GConsts.value option SMap.t ->
  VersionedSSet.diff ->
  Fanout.t * int

val get_modules_deps :
  ctx:Provider_context.t ->
  old_modules:Modules.value option SMap.t ->
  modules:VersionedSSet.diff ->
  Fanout.t * int
