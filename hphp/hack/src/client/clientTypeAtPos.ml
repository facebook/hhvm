(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go result output_json =
  if output_json then
    let response =
      match result with
      | None -> (None, None)
      | Some (str, json) -> (Some str, Some json)
    in
    Nuclide_rpc_message_printer.(
      infer_type_response_to_json response |> print_json)
  else
    match result with
    | Some (str, _) -> print_endline str
    | None -> print_endline "(unknown)"
