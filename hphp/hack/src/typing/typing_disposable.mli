(*
 * Copyright (c) Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Typing code concerned with disposable types. *)

val is_disposable_class : Typing_env_types.env -> Decl_provider.Class.t -> bool

val is_disposable_visitor :
  Typing_env_types.env -> string option Type_visitor.locl_type_visitor

(** Does ty (or a type embedded in ty) implement IDisposable
 * or IAsyncDisposable, directly or indirectly?
 * Return Some class_name if it does, None if it doesn't.
 *)
val is_disposable_type :
  Typing_env_types.env -> Typing_defs.locl_ty -> string option

val enforce_is_disposable : Typing_env_types.env -> 'a * Aast.hint_ -> unit
