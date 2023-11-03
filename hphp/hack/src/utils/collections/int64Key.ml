(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = int64 [@@deriving yojson_of]

let compare = Int64.compare

let to_string = Int64.to_string
