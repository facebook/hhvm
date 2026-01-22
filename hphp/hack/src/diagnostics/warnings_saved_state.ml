(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type error_hash = int [@@deriving ord, show]

module ErrorHash = struct
  type t = error_hash [@@deriving ord, show]
end

module ErrorHashSet = struct
  include Stdlib.Set.Make (ErrorHash)

  let pp fmt t = Format.pp_print_list ErrorHash.pp fmt (elements t)
end

include ErrorHashSet

type path = string

let read_from_disk path =
  Marshal.from_channel (In_channel.create ~binary:true path)
