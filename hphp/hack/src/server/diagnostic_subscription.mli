(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Reordered_argument_collections

type t

val of_id : id:int -> init:Errors.t -> t

val get_id : t -> int

val mark_as_pushed : t -> t

val update : t -> FileInfo.fast -> Errors.t -> t

(* Errors ready for sending to client *)
val get_absolute_errors : t -> (Pos.absolute Errors.error_ list) SMap.t
