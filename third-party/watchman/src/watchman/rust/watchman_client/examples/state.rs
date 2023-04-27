/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::path::PathBuf;

use structopt::StructOpt;
use watchman_client::prelude::*;

#[derive(Debug, StructOpt)]
#[structopt(about = "Exercise the state-enter and state-leave commands")]
struct Opt {
    #[structopt(default_value = ".")]
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
    let opt = Opt::from_args();
    let client = Connector::new().connect().await?;
    let resolved = client
        .resolve_root(CanonicalPath::canonicalize(opt.path)?)
        .await?;

    println!("resolved watch to {:?}", resolved);

    client
        .state_enter(&resolved, "woot", SyncTimeout::Default, None)
        .await?;
    println!("asserted woot state");
    tokio::time::sleep(std::time::Duration::from_secs(10)).await;
    client
        .state_leave(&resolved, "woot", SyncTimeout::Default, None)
        .await?;
    println!("vacated woot state");

    Ok(())
}
