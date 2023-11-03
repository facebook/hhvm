(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include WrappedMap.Make (LowerStringKey)

let pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit =
 (fun pp_data -> make_pp Format.pp_print_string pp_data)

let yojson_of_t yojson_of_v t =
  make_yojson_of_t LowerStringKey.to_string yojson_of_v t

(* Avoids warning 66 about unused open Ppx_yojson_conv_lib.Yojson_conv.Primitives *)
let _ = yojson_of_unit
