(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
include WrappedMap.Make (Int64Key)

let pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit =
 fun pp_data ->
  make_pp (fun fmt s -> Format.fprintf fmt "%S" (Int64.to_string s)) pp_data

let show pp_data x = Format.asprintf "%a" (pp pp_data) x

let yojson_of_t yojson_of_value t =
  make_yojson_of_t Int64Key.to_string yojson_of_value t
