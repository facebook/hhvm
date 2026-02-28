(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = int [@@deriving eq, hash, ord, show]

val track_names : bool ref

val tmp : unit -> t

val to_string : t -> string

val make : string -> t
