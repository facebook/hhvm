(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type depgraph_load_error =
  | Depgraph_not_found of string
  | Depgraph_open_error of string
  | Depgraph_invalid_mode of string
  | Depgraph_not_loaded
