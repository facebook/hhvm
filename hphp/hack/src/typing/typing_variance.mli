(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Check a class definition for correct usage of variant generic parameters *)
val class_def : Typing_env_types.env -> Nast.class_ -> unit

(** Check a type definition for correct usage of variant generic parameters *)
val typedef : Typing_env_types.env -> Nast.typedef -> unit

(** Get (positive,negative) occurrences of generic parameters in a type.
 * Only collect those present in tracked. If is_mutable is true, treat the
 * type as invariant, i.e. all type parameters appear both positively and negatively.
 *)
val get_positive_negative_generics :
  tracked:SSet.t ->
  is_mutable:bool ->
  Typing_env_types.env ->
  Typing_reason.decl_t list SMap.t * Typing_reason.decl_t list SMap.t ->
  Typing_defs.decl_ty ->
  Typing_reason.decl_t list SMap.t * Typing_reason.decl_t list SMap.t
