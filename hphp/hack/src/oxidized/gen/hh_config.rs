// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a3ddc1e34c80b273795762adb15c4469>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct HhConfig {
    pub version: Option<String>,
    pub ignored_paths: Vec<String>,
    pub hash: String,
    pub opts: global_options::GlobalOptions,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub gc_minor_heap_size: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub gc_space_overhead: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub sharedmem_global_size: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub sharedmem_hash_table_pow: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub sharedmem_heap_size: usize,
    pub ide_fall_back_to_full_index: bool,
    pub hh_distc_should_disable_trace_store: bool,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub hh_distc_exponential_backoff_num_retries: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub naming_table_compression_level: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub naming_table_compression_threads: usize,
    #[rust_to_ocaml(attr = "oxidize.usize")]
    pub eden_fetch_parallelism: usize,
    pub use_distc_crawl_dircache: bool,
}
