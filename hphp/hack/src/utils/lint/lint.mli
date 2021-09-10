(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val internal_error : Pos.t -> string -> unit

val lowercase_constant : Pos.t -> string -> unit

val mk_lowercase_constant : Pos.t -> string -> Pos.t Lints_core.t

val use_collection_literal : Pos.t -> string -> unit

val static_string : ?no_consts:bool -> Pos.t -> unit

val shape_idx_access_required_field : Pos.t -> string -> unit

val opt_closed_shape_idx_missing_field : string option -> Pos.t -> unit
