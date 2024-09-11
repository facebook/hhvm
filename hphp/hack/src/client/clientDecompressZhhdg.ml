(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  path: string;
  from: string;
}

let main (env : env) : Exit_status.t =
  let deptable_result =
    Depgraph_decompress_ffi.decompress ~compressed_dg_path:env.path
  in
  match deptable_result with
  | Ok decompressed_path ->
    Printf.printf "Decompressed depgraph to %s\n" decompressed_path;
    Exit_status.No_error
  | Error err ->
    failwith (Printf.sprintf "Failed to decompress dep graph: %s" err)
