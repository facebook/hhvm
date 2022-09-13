// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg_attr(not(fbcode_build), allow(unused_variables))]

use ocamlrep_caml_builtins::Int64;
use ocamlrep_custom::Custom;
use rpds::HashTrieSet;

#[cfg(fbcode_build)]
pub struct HhFanoutRustFfi(hh_fanout_lib::HhFanoutInproc);
#[cfg(not(fbcode_build))]
pub struct HhFanoutRustFfi;

impl ocamlrep_custom::CamlSerialize for HhFanoutRustFfi {
    ocamlrep_custom::caml_serialize_default_impls!();
}

#[derive(ocamlrep_derive::ToOcamlRep, ocamlrep_derive::FromOcamlRep)]
#[repr(C)]
enum EdgesError {
    EeDecl(String),
}

type EdgesResult<T> = Result<T, EdgesError>;

/// Rust set of edges.
#[derive(Debug)]
pub struct EdgesBuffer(HashTrieSet<hh24_types::DepgraphEdge>);

impl std::ops::Deref for EdgesBuffer {
    type Target = HashTrieSet<hh24_types::DepgraphEdge>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl From<HashTrieSet<hh24_types::DepgraphEdge>> for EdgesBuffer {
    fn from(set: HashTrieSet<hh24_types::DepgraphEdge>) -> Self {
        Self(set)
    }
}

impl ocamlrep_custom::CamlSerialize for EdgesBuffer {
    ocamlrep_custom::caml_serialize_default_impls!();
}

ocamlrep_ocamlpool::ocaml_ffi! {
 fn hh_fanout_start_edges(hh_fanout: Custom<HhFanoutRustFfi>) -> Custom<EdgesBuffer> {
     // TODO(toyang): do things with hh_fanout.0
     let _ = hh_fanout;
     todo!()
 }

 fn hh_fanout_add_edge(hh_fanout: Custom<HhFanoutRustFfi>, dependent: Int64, dependency: Int64, buffer: Custom<EdgesBuffer>, expected_checksum: Int64) -> EdgesResult<Custom<EdgesBuffer>> {
     let _ = (hh_fanout, dependent, dependency, buffer, expected_checksum);
     todo!()
 }

 fn hh_fanout_finish_edges(hh_fanout: Custom<HhFanoutRustFfi>, buffer: Custom<EdgesBuffer>) -> EdgesResult<()> {
     let _ =(hh_fanout, buffer);
     todo!()
 }
}
