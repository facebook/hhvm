(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let print_json res =
  Nuclide_rpc_message_printer.(outline_response_to_json res |> print_json)

let go res output_json =
  if output_json then
    print_json res
  else
    FileOutline.print res
