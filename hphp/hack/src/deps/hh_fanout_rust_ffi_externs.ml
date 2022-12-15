(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type hh_fanout_rust_ffi

type depgraph_edge_ffi

(* Keep order of arguments consistent with Rust FFI! *)

external make :
  fanout_state_dir:string -> decl_state_dir:string -> hh_fanout_rust_ffi
  = "hh_fanout_ffi_make"

external make_hhdg_builder : builder_state_dir:string -> hh_fanout_rust_ffi
  = "hh_fanout_ffi_make_hhdg_builder"

(* The list is of (dependency, dependent) *)
external commit_edges : hh_fanout_rust_ffi -> (int * int) list -> unit
  = "hh_fanout_ffi_add_idep_batch"
