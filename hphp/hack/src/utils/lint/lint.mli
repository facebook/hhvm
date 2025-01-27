(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val internal_error : Pos.t -> string -> unit

val use_collection_literal : Pos.t -> string -> unit

val static_string : ?no_consts:bool -> Pos.t -> unit

val shape_idx_access_required_field : Pos.t -> string -> unit

val sealed_not_subtype : string -> Pos.t -> string -> string -> string -> unit

val option_mixed : Pos.t -> unit

val option_null : Pos.t -> unit

val class_const_to_string : Pos.t -> string -> unit
