/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! This example shows how to send a query to watchman and print out the files
//! changed since the given timestamp.

use std::path::PathBuf;

use clap::Parser;
use watchman_client::prelude::*;

/// Query files changed since a timestamp
#[derive(Debug, Parser)]
struct Cli {
    /// Specifies the clock. Use `watchman clock <PATH>` to retrieve the current
    /// clock of a watched directory
    clock: String,

    #[arg(short, long)]
    /// [not recommended] Uses Unix timestamp as clock
    unix_timestamp: bool,

    #[arg(short, long, default_value = ".")]
    /// Specifies the path to watched directory
    path: PathBuf,
}

#[tokio::main(flavor = "current_thread")]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    if let Err(err) = run().await {
        // Print a prettier error than the default
        eprintln!("{}", err);
        std::process::exit(1);
    }
    Ok(())
}

async fn run() -> Result<(), Box<dyn std::error::Error>> {
    let opt = Cli::parse();
    let client = Connector::new().connect().await?;
    let resolved = client
        .resolve_root(CanonicalPath::canonicalize(opt.path)?)
        .await?;

    let clock_spec = if opt.unix_timestamp {
        // it is better to use watchman's clock rather than Unix timestamp.
        // see `watchman_client::pdu::ClockSpec::unix_timestamp` for details.
        ClockSpec::UnixTimestamp(opt.clock.parse()?)
    } else {
        ClockSpec::StringClock(opt.clock)
    };

    let result = client
        .query::<NameOnly>(
            &resolved,
            QueryRequestCommon {
                since: Some(Clock::Spec(clock_spec.clone())),
                ..Default::default()
            },
        )
        .await?;

    eprintln!("Clock is now: {:?}", result.clock);

    if let Some(files) = result.files {
        for file in files.iter() {
            println!("{}", file.name.display());
        }
    } else {
        eprintln!("no file changed since {:?}", clock_spec);
    }

    Ok(())
}
