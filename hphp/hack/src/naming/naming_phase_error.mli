(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

type agg

val naming : Naming_error.t -> t

val nast_check : Nast_check_error.t -> t

val typing : Typing_error.Primary.t -> t

val like_type : Pos.t -> t

val unexpected_hint : Pos.t -> t

val malformed_access : Pos.t -> t

val supportdyn : Pos.t -> t

val invalid_expr_ : Pos.t -> (unit, unit) Aast.expr_

val add : agg -> t -> agg

val emit : agg -> unit

val empty : agg
