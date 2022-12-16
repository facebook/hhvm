// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[cfg(fbcode_build)]
use std::path::Path;
use std::path::PathBuf;

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
    fn hh_fanout_ffi_make(fanout_state_dir: PathBuf, decl_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        // TODO(toyang): we should replace this log with a real scuba logger and
        // a file log in the state dir or in a configured location. See
        // hh_decl_ffi.rs for another location we need to pass a better log
        // object.
        let log = hh_slog::Log {
            file: hh_slog::init_file_sync(Path::new("/tmp/hh_fanout_log")),
            scuba: hh_slog::init_file_sync(Path::new("/tmp/hh_fanout_log_scuba")),
        };
        let hh_decl = Box::new(hh_decl_shmem::DeclShmem::new(log.clone(), decl_state_dir));
        let hh_fanout = hh_fanout_lib::HhFanoutImpl::new(log, fanout_state_dir, hh_decl);
        Custom::from(HhFanoutRustFfi(std::cell::RefCell::new(Box::new(hh_fanout))))
    }

    fn hh_fanout_ffi_make_hhdg_builder(builder_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        // TODO(toyang): we should replace this log with a real scuba logger and
        // a file log in the state dir or in a configured location. See
        // hh_decl_ffi.rs for another location we need to pass a better log
        // object.
        let log = hh_slog::Log {
            file: hh_slog::init_file_sync(Path::new("/tmp/hh_fanout_log")),
            scuba: hh_slog::init_file_sync(Path::new("/tmp/hh_fanout_log_scuba")),
        };
        let hhdg_builder = hhdg_builder::HhdgBuilder::new(log, builder_state_dir);
        Custom::from(HhFanoutRustFfi(std::cell::RefCell::new(Box::new(hhdg_builder))))
    }

    // Each edge is a tuple of (dependency, dependent).
    fn hh_fanout_ffi_add_idep_batch(hh_fanout: Custom<HhFanoutRustFfi>, edges: Vec<(u64, u64)>) {
        use hh24_types::DepGraphEdge;
        if let Err(err) = hh_fanout.0.borrow_mut().commit_edges(
            edges.into_iter().map(|(dependency, dependent)| DepGraphEdge::from_u64(dependency, dependent)).collect()
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
    fn hh_fanout_ffi_make_hhdg_builder(builder_state_dir: PathBuf) -> Custom<HhFanoutRustFfi> {
        unimplemented!()
    }
    fn hh_fanout_ffi_add_idep_batch(_hh_fanout: Custom<HhFanoutRustFfi>, _edges: Vec<(u64, u64)>) {
        unimplemented!()
    }
}
