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
include Set.Make (Char)

let yojson_of_t t =
  elements t |> List.sort Char.compare |> yojson_of_list yojson_of_char
