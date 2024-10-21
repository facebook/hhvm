(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'pos elem =
  | Witness of ('pos[@hash.ignore]) * string
  | Witness_no_pos of string
  | Rule of string
  | Step of string * bool
  | Trans of string
  | Prefix of {
      prefix: string;
      sep: string;
    }
  | Suffix of {
      suffix: string;
      sep: string;
    }
[@@deriving eq, hash, ord, show]

type 'pos t =
  | Derivation of 'pos elem list
  | Debug of string
  | Empty
[@@deriving eq, hash, ord, show]

val debug : string -> 'a t

val derivation : 'a elem list -> 'a t

val empty : 'a t

val map : 'a t -> f:('a -> 'b) -> 'b t
