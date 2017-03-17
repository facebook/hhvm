(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let go ty output_json =
  if output_json
  then begin
    let response = Ide_message.Infer_type_response ty in
    Nuclide_rpc_message_printer.print_json ~response
  end else begin
    match ty with
      | Some ty -> print_endline ty
      | None -> print_endline "(unknown)"
  end
