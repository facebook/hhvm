(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

type t =
  | Mismatch of Patt_binding_ty.t * Patt_binding_ty.t
  | Shadowed of Patt_var.t
  | Unbound of Patt_var.t
[@@deriving compare, eq, sexp, show]
