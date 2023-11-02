// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(clippy::todo)]

use std::path::PathBuf;

use dep::Dep;
use ocamlrep_custom::Custom;

#[cfg(fbcode_build)]
pub struct HhFanoutRustFfi(std::cell::RefCell<Box<dyn hh_fanout_api::HhFanoutEdgeAccumulator>>);
#[cfg(not(fbcode_build))]
pub struct HhFanoutRustFfi;

impl ocamlrep_custom::CamlSerialize for HhFanoutRustFfi {
    ocamlrep_custom::caml_serialize_default_impls!();
}

#[cfg(fbcode_build)]
ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_fanout_ffi_make(logger: Custom<file_scuba_logger_ffi::FileScubaLoggerFfi>, fanout_state_dir: PathBuf, decl_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        let hh_decl = Box::new(hh_decl_shmem::DeclShmem::new(logger.0.clone(), decl_state_dir));
        let hh_fanout = hh_fanout_lib::HhFanoutImpl::new(logger.0.clone(), fanout_state_dir, hh_decl);
        Custom::from(HhFanoutRustFfi(std::cell::RefCell::new(Box::new(hh_fanout))))
    }

    fn hh_fanout_ffi_make_hhdg_builder(logger: Custom<file_scuba_logger_ffi::FileScubaLoggerFfi>, builder_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        let hhdg_builder = hhdg_builder::HhdgBuilder::new(logger.0.clone(), builder_state_dir);
        Custom::from(HhFanoutRustFfi(std::cell::RefCell::new(Box::new(hhdg_builder))))
    }

    // Each edge is a tuple of (dependency, dependent).
    fn hh_fanout_ffi_add_idep_batch(hh_fanout: Custom<HhFanoutRustFfi>, edges: Vec<(Dep, Dep)>) {
        // TODO: the conversion of Vec<(Dep, Dep)> to DepGraphEdge probably
        // isn't too efficient. The construction of the Rust Vec from OCaml list
        // might also be inefficient. Left for later optimization.
        if let Err(err) = hh_fanout.0.borrow_mut().commit_edges(
            edges.into_iter().map(|(dependency, dependent)| hh24_types::DepGraphEdge {
                dependency: hh24_types::DependencyHash(dependency.into()),
                dependent: hh24_types::ToplevelSymbolHash::from_u64(dependent.into()) }).collect()
        ) {
            eprintln!("Error: {err}");
            todo!("deal with hh errors like checksum mismatch");
        };
    }
}

#[cfg(not(fbcode_build))]
// This FFI only works for fbcode builds at the moment, due to trickiness with
// dune working with cargo and not playing well with some dependencies.
ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_fanout_ffi_make(_fanout_state_dir: PathBuf, _decl_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        unimplemented!()
    }
    fn hh_fanout_ffi_make_hhdg_builder(_builder_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        unimplemented!()
    }
    fn hh_fanout_ffi_add_idep_batch(_hh_fanout: Custom<HhFanoutRustFfi>, _edges: Vec<(Dep, Dep)>) {
        unimplemented!()
    }
}
