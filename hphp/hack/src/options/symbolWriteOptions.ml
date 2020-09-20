(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = GlobalOptions.t [@@deriving show]

let root_path = GlobalOptions.symbol_write_root_path

let hhi_path = GlobalOptions.symbol_write_hhi_path

let ignore_paths = GlobalOptions.symbol_write_ignore_paths

let index_paths = GlobalOptions.symbol_write_index_paths

let include_hhi = GlobalOptions.symbol_write_include_hhi

let default = GlobalOptions.default
