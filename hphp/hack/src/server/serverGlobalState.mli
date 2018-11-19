(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t
val save : logging_init:(unit -> unit) -> t
val restore : t -> unit

val to_string : t -> string
