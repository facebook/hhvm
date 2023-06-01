(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Config of Custom_error.t list
[@@ocaml.unboxed] [@@deriving show, yojson]

val empty : t

val initialize : string -> (t * string list, string) result
