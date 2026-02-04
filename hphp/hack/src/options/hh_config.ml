(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(**
 * OCaml representation of hh_config::HhConfig from Rust.
 * IMPORTANT: Field order must match the Rust struct exactly for FFI.
 *)

type t = {
  version: string option;
  ignored_paths: string list;
  hash: string;
  opts: GlobalOptions.t;
  gc_minor_heap_size: int;
  gc_space_overhead: int;
  hackfmt_version: int;
  sharedmem_dep_table_pow: int;
  sharedmem_global_size: int;
  sharedmem_hash_table_pow: int;
  sharedmem_heap_size: int;
  ide_fall_back_to_full_index: bool;
  hh_distc_should_disable_trace_store: bool;
  hh_distc_exponential_backoff_num_retries: int;
  naming_table_compression_level: int;
  naming_table_compression_threads: int;
  eden_fetch_parallelism: int;
  use_distc_crawl_dircache: bool;
  distc_avoid_unnecessary_saved_state_work: bool;
  distc_write_trace_during_save_state_creation_only: bool;
}
[@@deriving eq, show]
