(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Ppx_yojson_conv_lib.Yojson_conv.Primitives

type t = string [@@deriving yojson_of]

let compare (x : t) (y : t) = String.compare x y

let to_string x = x
