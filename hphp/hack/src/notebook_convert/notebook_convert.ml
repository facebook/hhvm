(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let notebook_to_hack ~(notebook_name : string) : Exit_status.t =
  let () =
    Printf.printf
      "TODO: convert notebook to Hack. notebook_name: %s\n"
      notebook_name
  in
  Exit_status.No_error
