(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val is_www_directory : ?config:string -> Path.t -> bool

val assert_www_directory : ?config:string -> Path.t -> unit

val get : ?config:string -> string option -> Path.t
