(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* TODO(T170647909): In preparation to upgrading to ppx_yojson_conv.v0.16.X.
         Remove the suppress warning when the upgrade is done. *)
[@@@warning "-66"]

open Ppx_yojson_conv_lib.Yojson_conv.Primitives

type t = int [@@deriving yojson_of]

let compare = Int.compare

let to_string = Int.to_string
