(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val add : int -> Pos.t -> string -> unit

val lowercase_constant : Pos.t -> string -> unit
val use_collection_literal : Pos.t -> string -> unit
val single_quoted_string : Pos.t -> unit

val do_ : (unit -> 'a) -> Errors.error list * 'a
