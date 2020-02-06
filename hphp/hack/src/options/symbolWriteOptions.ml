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

let default = GlobalOptions.default
