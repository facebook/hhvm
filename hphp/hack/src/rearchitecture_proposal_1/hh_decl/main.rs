// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! hh_decl is the "local decl service", a singleton-per-root process.
//! It owns the decl cachelib, the naming table, and the prototype process.
//! It listens to /tmp/hh_server/[root].decl.sock for decl requests
//! It listens to /tmp/hh_server/[root].decl.control.sock for control requests

use bytes::Bytes;
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};
use std::thread::JoinHandle;
use structopt::StructOpt;

mod control_service;
mod decl_service;
mod hh_tmpdir;
mod prototype;

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

#[fbinit::main]
fn main(fb: fbinit::FacebookInit) {
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

    // Spin up cachelib
    let cache_size = 1 * 1024 * 1024 * 1024; // 1 GB
    let cache_directory = hh_tmpdir::tmp_path_of_root(&args.root, ".decl.cache");
    let _ = nix::unistd::unlink(&cache_directory);
    let cache_config = cachelib::LruCacheConfig::new(cache_size).set_cache_dir(&cache_directory);
    cachelib::init_cache_once(fb, cache_config).unwrap();
    cachelib::init_cacheadmin("hh_decl_cache").unwrap();
    let available_space = cachelib::get_available_space().unwrap();
    let default_pool = cachelib::get_or_create_pool("default", available_space).unwrap();

    // Here's just a dummy entry in the cachelib so I can test retrieving it later
    let key = Bytes::from(&b"hello"[..]);
    let value = Bytes::from(&b"world"[..]);
    default_pool.set(&key, value).unwrap();

    // TODO: cancellation tokens for graceful shutdown

    // Create control socket
    let control_sock_file = hh_tmpdir::tmp_path_of_root(&args.root, ".decl.control.sock");
    let _ = nix::unistd::unlink(&control_sock_file); // ignored because failure will be reported later upon bind
    let control_listener = match std::os::unix::net::UnixListener::bind(&control_sock_file) {
        Err(e) => {
            println!("failed to listen on {:?} - {:?}", &control_sock_file, e);
            std::process::exit(1);
        }
        Ok(listener) => listener,
    };

    // Create decl socket
    let decl_sock_file = hh_tmpdir::tmp_path_of_root(&args.root, ".decl.sock");
    let _ = nix::unistd::unlink(&decl_sock_file); // ignored because failure will be reported later upon bind
    let decl_listener = match std::os::unix::net::UnixListener::bind(&decl_sock_file) {
        Err(e) => {
            println!("failed to listen on {:?} - {:?}", &decl_sock_file, e);
            std::process::exit(1);
        }
        Ok(listener) => listener,
    };

    // Spin up prototype
    let _base_address = match prototype::spawn(&args.root, &decl_sock_file, &cache_directory) {
        Err(e) => {
            println!("failed to launch prototype: {:?}", e);
            std::process::exit(1);
        }
        Ok(r) => r,
    };

    // Spin up control service thread.
    // It will handle all connections sequentially.
    let control_thread = std::thread::spawn(move || {
        for stream in control_listener.incoming() {
            match stream {
                Err(e) => println!("decl control: malformed {:?}; continuing", e),
                Ok(stream) => control_service::handle(stream),
            };
        }
    });

    // Spin up decl service thread.
    // It will spin up a new thread for each connection.
    // It will wait until all those threads have finished.
    let decl_thread = std::thread::spawn(move || {
        let mut worker_id = 0u64;
        let worker_threads = Arc::new(Mutex::new(HashMap::<u64, JoinHandle<()>>::new()));
        for stream in decl_listener.incoming() {
            match stream {
                Err(e) => println!("decl: malformed {:?}; continuing", e),
                Ok(stream) => {
                    worker_id += 1;
                    let worker_threads2 = worker_threads.clone();
                    let worker_id2 = worker_id;
                    let worker_thread = std::thread::spawn(move || {
                        decl_service::handle(stream);
                        worker_threads2.lock().unwrap().remove(&worker_id2);
                    });
                    worker_threads
                        .lock()
                        .unwrap()
                        .insert(worker_id, worker_thread);
                }
            }
        }
        // We'll wait until all threads have finished.
        // We don't want to be holding the mutex as we iter over those threads
        // (since the threads need the mutex to finish up). Hence, drain.
        let mut locked_hashmap = worker_threads.lock().unwrap();
        let remaining_threads = locked_hashmap.drain();
        for (_, remaining_thread) in remaining_threads {
            remaining_thread.join().unwrap();
        }
    });

    control_thread.join().unwrap();
    decl_thread.join().unwrap();

    std::mem::drop(lock);
}
