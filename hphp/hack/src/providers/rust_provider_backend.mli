(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val make : ParserOptions.t -> t

module Decl : sig
  val get_fun : t -> string -> Shallow_decl_defs.fun_decl option

  val get_shallow_class : t -> string -> Shallow_decl_defs.class_decl option

  val get_typedef : t -> string -> Shallow_decl_defs.typedef_decl option

  val get_gconst : t -> string -> Shallow_decl_defs.const_decl option

  val get_module : t -> string -> Shallow_decl_defs.module_decl option

  val get_folded_class : t -> string -> Decl_defs.decl_class_type option

  val push_local_changes : t -> unit

  val pop_local_changes : t -> unit
end
