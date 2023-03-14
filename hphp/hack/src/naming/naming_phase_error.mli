(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

type agg

exception UnexpectedExpr of Pos.t

val naming : Naming_error.t -> t

val parsing : Parsing_error.t -> t

val nast_check : Nast_check_error.t -> t

val like_type : Pos.t -> t

val unexpected_hint : Pos.t -> t

val malformed_access : Pos.t -> t

val supportdyn : Pos.t -> t

val invalid_expr_ : ('ex, 'en) Aast.expr option -> ('ex, 'en) Aast.expr_

val invalid_expr : ('ex, 'en) Aast.expr -> ('ex, 'en) Aast.expr

val add : agg -> t -> agg

val emit : agg -> unit

val empty : agg
