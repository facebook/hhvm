(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

type t = GlobalOptions.t [@@deriving show]

let root_path t = t.GlobalOptions.symbol_write_root_path

let hhi_path t = t.GlobalOptions.symbol_write_hhi_path

let ignore_paths t = t.GlobalOptions.symbol_write_ignore_paths

let index_paths t = t.GlobalOptions.symbol_write_index_paths

let include_hhi t = t.GlobalOptions.symbol_write_include_hhi

let ownership t = t.GlobalOptions.symbol_write_ownership

let default = GlobalOptions.default
