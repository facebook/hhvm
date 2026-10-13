(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(**
 * OCaml type returned by the Rust config parser equivalence test.
 * The Rust representation is generated from this type by hh_oxidize.
 *)

type t = {
  version: string option;
  ignored_paths: string list;
  hash: string;
  opts: Global_options.t;
  gc_minor_heap_size: int; [@oxidize.usize]
  gc_space_overhead: int; [@oxidize.usize]
  sharedmem_global_size: int; [@oxidize.usize]
  sharedmem_hash_table_pow: int; [@oxidize.usize]
  sharedmem_heap_size: int; [@oxidize.usize]
  ide_fall_back_to_full_index: bool;
  hh_distc_should_disable_trace_store: bool;
  hh_distc_exponential_backoff_num_retries: int; [@oxidize.usize]
  naming_table_compression_level: int; [@oxidize.usize]
  naming_table_compression_threads: int; [@oxidize.usize]
  eden_fetch_parallelism: int; [@oxidize.usize]
  use_distc_crawl_dircache: bool;
}
[@@deriving eq, show]
