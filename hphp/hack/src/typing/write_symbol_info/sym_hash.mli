(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Set of Md5.t hashes *)
type t

val mem : t -> Md5.t -> bool

(** Construct a [t] from file [path]. File is a string of concatenated
    base64-encoded md5 hash. Fails if file doesn't exist or unable to
    deserialize *)
val read : path:string -> t
