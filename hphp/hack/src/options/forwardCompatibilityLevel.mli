(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | OSS_3_26
  | OSS_3_27
  | OSS_3_28
  | HEAD
  | ISODate of int (* yyyymmdd *)
  [@@deriving show]

val default: t
val current: t
val minimum: t

val from_string: string -> t
val as_int: t -> int
val as_string: t -> string
val greater_than: t -> t -> bool
