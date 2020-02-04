(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [Shallow_classes_heap] provides a cache of shallow class declarations when
    the local config option [shallow_class_decl] is enabled. When it is not
    enabled, this module provides only [class_naming_and_decl], which converts
    an AST to a shallow declaration, and other functions raise an exception. *)

open Shallow_decl_defs

(** Return the shallow declaration of the class with the given name if it is
    present in the cache. Otherwise, compute it, store it in the cache, and
    return it.

    Raises [Failure] if [shallow_class_decl] is not enabled. *)
val get : Provider_context.t -> string -> shallow_class option

(** Convert the given class AST to a shallow class declaration and return it. *)
val class_naming_and_decl : Provider_context.t -> Nast.class_ -> shallow_class

(** If a shallow declaration for the class with the given name is present in the
    cache, return it. Otherwise, convert the given class AST to a shallow class
    declaration, store it in the cache, and return it.

    Raises [Failure] if [shallow_class_decl] is not enabled. *)
val class_decl_if_missing : Provider_context.t -> Nast.class_ -> shallow_class

val declare_class_in_file :
  Provider_context.t -> Relative_path.t -> string -> shallow_class

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit

val get_batch : SSet.t -> shallow_class option SMap.t

val get_old_batch : SSet.t -> shallow_class option SMap.t

val oldify_batch : SSet.t -> unit

val remove_old_batch : SSet.t -> unit

val remove_batch : SSet.t -> unit
