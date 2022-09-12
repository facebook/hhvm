(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Checks if a type is one that HHVM will enforce as a parameter or return *)
val is_enforceable : Provider_context.t -> Typing_defs.decl_ty -> bool

(** If the type is not enforceable, turn it into a like type (~ty) otherwise
    return the type *)
val pessimise_type :
  Provider_context.t -> Typing_defs.decl_ty -> Typing_defs.decl_ty

(** Pessimise the type if in implicit pessimisation mode, otherwise
    return the type *)
val maybe_pessimise_type :
  Provider_context.t -> Typing_defs.decl_ty -> Typing_defs.decl_ty

(** If the return type is not enforceable, turn it into a like type (~ty) otherwise
    return the original function type *)
val pessimise_fun_type :
  Provider_context.t -> Typing_defs.decl_ty -> Typing_defs.decl_ty

(** Pessimise the type if in implicit pessimisation mode, otherwise
    return the type *)
val maybe_pessimise_fun_type :
  Provider_context.t -> Typing_defs.decl_ty -> Typing_defs.decl_ty
