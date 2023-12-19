(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = int ref

module Ident = struct
  type t = int [@@deriving ord, eq, hash, show]

  module Map = WrappedMap.Make (Int)
  module Set = Stdlib.Set.Make (Int)

  let immutable_mask = 1 lsl 62

  let is_immutable i = i land immutable_mask <> 0

  let make_immutable i = i lor immutable_mask

  let dodgy_from_int i = i
end

let init () = ref 0

let provide provider =
  let id = !provider in
  provider := !provider + 1;
  id
