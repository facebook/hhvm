(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type error = (Pos.t * string) list
type t = error list

val add: Pos.t -> string -> unit
val add_list: error -> unit

val do_: (unit -> 'a) -> t * 'a
val try_: (unit -> 'a) -> (error -> 'a) -> 'a
val to_json: error -> Json.json
val pmsg_l: error -> string
val to_string: error -> string
