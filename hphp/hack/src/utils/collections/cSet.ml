(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Set.Make (Char)

let yojson_of_t t =
  elements t |> List.sort Char.compare |> yojson_of_list yojson_of_char
