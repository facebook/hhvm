(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

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

let map_elem elem ~f =
  match elem with
  | Witness (pos, str) -> Witness (f pos, str)
  | Witness_no_pos str -> Witness_no_pos str
  | Rule rule -> Rule rule
  | Step (path, is_error) -> Step (path, is_error)
  | Trans trans -> Trans trans
  | Prefix pfx -> Prefix pfx
  | Suffix sfx -> Suffix sfx

type 'pos t =
  | Derivation of 'pos elem list
  | Debug of string
  | Empty
[@@deriving eq, hash, ord, show]

let map t ~f =
  match t with
  | Debug str -> Debug str
  | Derivation elems -> Derivation (List.map elems ~f:(map_elem ~f))
  | Empty -> Empty

let derivation elems = Derivation elems

let debug str = Debug str

let empty = Empty
