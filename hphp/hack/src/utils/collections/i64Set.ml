(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Ppx_yojson_conv_lib.Yojson_conv.Primitives
include Set.Make (Int64)

let yojson_of_t t =
  elements t |> List.sort Int64.compare |> yojson_of_list yojson_of_int64
