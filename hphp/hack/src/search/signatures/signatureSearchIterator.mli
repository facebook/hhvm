(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val empty : t

val from_file : string -> t

val from_list : int list -> t

val make_and : t -> t -> t

val make_or : t -> t -> t

val make_diff : t -> t -> t

val next : t -> int option
