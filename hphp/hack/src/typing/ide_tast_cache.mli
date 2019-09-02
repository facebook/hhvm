(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val enable : unit -> unit

val get : Relative_path.t -> string -> (unit -> Tast.program) -> Tast.program

val invalidate : unit -> unit
