(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_message

let go result output_json =
  if output_json
  then begin
    let response =
      (match result with
        | None -> Infer_type_response
          { type_string = None; type_json = None }
        | Some (str, json) -> Infer_type_response
          { type_string = Some str; type_json = Some json }
      ) in
    Nuclide_rpc_message_printer.print_json ~response
  end else begin
    match result with
      | Some (str, _) -> print_endline str
      | None -> print_endline "(unknown)"
  end
