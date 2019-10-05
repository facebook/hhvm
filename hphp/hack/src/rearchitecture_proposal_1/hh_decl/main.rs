// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! hh_decl is the "local decl service" as described in the architecture
//! writeup. It is responsible for owning the decl cache and exposes two
//! service endpoints:
//!  1) The control service, which receives requests to update the naming table
//!     and requests for status,
//!  2) The decl service, which serves decls to typecheck clients.
//!

use control_service::ControlService;
use decl_service::DeclService;
use server::UnixDomainServer;
use std::path::PathBuf;
use std::thread;
use structopt::StructOpt;

mod control_service;
mod decl_service;
mod hh_tmpdir;
mod server;

/// The well-known socket that the control service listens on.
const CONTROL_SERVICE_SOCKET: &'static str = "/tmp/hh_server/control_service";

/// The well-known socket that the decl service listens on.
const DECL_SERVICE_SOCKET: &'static str = "/tmp/hh_server/decl_service";

/// CommandLineArgs declaratively defines the command-line interface of hh_decl.
/// Structopt is responsible for deriving a command-line parser from this
/// structure.
#[derive(Debug, StructOpt)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (buck doesn't set it)
#[structopt(rename_all = "kebab")] // rename every option to kebab-case
struct CommandLineArgs {
    /// Root, where .hhconfig is
    #[structopt(long, parse(from_os_str))]
    root: PathBuf,

    /// Optional path to the naming table to use
    #[structopt(long, parse(from_os_str))]
    naming_table: Option<PathBuf>,

    /// Size of the cache
    #[structopt(long, default_value = "8192")]
    cachelib_size: u64,
}

fn main() {
    let args = CommandLineArgs::from_args();

    // TODO: validate root (canonicalize, make sure .hhconfig exists)

    // Check lock-file sentinel to ensure only one instance is running per root
    let lock_file = hh_tmpdir::tmp_path_of_root(&args.root, ".decl.lock");
    let lock = match hh_tmpdir::open_exclusive_lock_file(&lock_file).unwrap() {
        None => {
            println!("local decl service is already running");
            std::process::exit(1);
        }
        Some(fd) => fd,
    };
    println!("hh_decl starting...");

    // Spawn our two services: the decl service and control service. The control
    // service runs on the main thread (arbitrarily).
    thread::spawn(move || {
        let decl_svr: UnixDomainServer<DeclService> =
            UnixDomainServer::new(&PathBuf::from(DECL_SERVICE_SOCKET))
                .expect("failed to start decl service");
        decl_svr.start();
    });

    let svr: UnixDomainServer<ControlService> =
        UnixDomainServer::new(&PathBuf::from(CONTROL_SERVICE_SOCKET))
            .expect("failed to start control service");
    svr.start();

    std::mem::drop(lock);
}
