(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Check for the __NonDisjoint attribute on type parameters of functions and methods.
  All type parameters with this attribute should be infered to types which are not disjoint. *)
include Handler.Warning.S
