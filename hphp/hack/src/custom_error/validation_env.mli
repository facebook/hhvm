(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val empty : t

val add : t -> key:Patt_var.t -> data:Patt_binding_ty.t -> (t, t) result

val get : t -> Patt_var.t -> Patt_binding_ty.t option

val combine : t -> t -> t * (Validation_err.t list * Validation_err.t list)
