(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

type t =
  | Primary of primary
  | Apply of callback * t
  | Apply_reasons of reasons_callback * secondary
  | Or of t * t

and primary = Any_prim

and secondary =
  | Of_error of t
  | Violated_constraint of Patt_name.t * Patt_locl_ty.t * Patt_locl_ty.t
  | Subtyping_error of Patt_locl_ty.t * Patt_locl_ty.t
  | Any_snd

and callback = Any_callback

and reasons_callback = Any_reasons_callback [@@deriving show, yojson]
