// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Eden State Asserter - A tool for testing EdenFS state transitions
//!
//! This is used by test_edenfs_file_watcher.py to assert a state for the given
//! amount of time, then un-assert it.

use std::env;
use std::path::Path;
use std::process;
use std::thread;
use std::time::Duration;

use edenfs_asserted_states::StreamingChangesClient;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 4 {
        eprintln!(
            "Usage: {} <eden_mount_point> <state_name> <duration_secs> ",
            args[0]
        );
        process::exit(1);
    }

    let eden_mount_point = &args[1];
    let state_name = &args[2];
    let duration_secs: f64 = match args[3].parse() {
        Ok(num) => num,
        Err(_) => {
            eprintln!("Error: duration_secs must be a valid f64");
            process::exit(1);
        }
    };

    let eden_state_client = match StreamingChangesClient::new(
        Path::new(eden_mount_point),
        "hh_eden_test_state_asserter".to_string(),
    ) {
        Ok(client) => client,
        Err(e) => {
            eprintln!("Error creating StreamingChangesClient: {}", e);
            process::exit(1);
        }
    };

    let _guard = match eden_state_client.enter_state(state_name) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Error entering state '{}': {}", state_name, e);
            process::exit(1);
        }
    };

    println!("state asserted");

    thread::sleep(Duration::from_secs_f64(duration_secs));

    println!("state unasserted");
    // State is automatically un-asserted here when dropping _guard
}
