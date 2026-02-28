(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external decompress : compressed_dg_path:string -> (string, string) result
  = "depgraph_decompress_ffi"
