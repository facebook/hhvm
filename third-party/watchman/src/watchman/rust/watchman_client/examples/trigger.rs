/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! This example shows how to setup and remove a persistent trigger.

use std::path::Path;
use std::path::PathBuf;

use clap::Parser;
use watchman_client::prelude::*;

/// Interact with watchman triggers.
#[derive(Debug, Parser)]
enum Cli {
    /// Registers a watcher trigger that will stream the list of files modified.
    Register {
        /// Specifies the path to watched directory
        path: PathBuf,

        /// Specifies the name of the trigger to register.
        name: String,

        /// Specifies the output file, must be prefixed with `>` or `>>`.
        output_file: String,
    },
    /// Removes a watcher trigger.
    Del {
        /// Specifies the path to watched directory
        path: PathBuf,

        /// Specifies the name of the trigger to remove.
        name: String,
    },
    /// Lists all watcher triggers for a path.
    List {
        /// Specifies the path to watched directory
        path: PathBuf,
    },
}
impl Cli {
    fn path(&self) -> &Path {
        match self {
            Self::Register { path, .. } => path,
            Self::Del { path, .. } => path,
            Self::List { path, .. } => path,
        }
    }
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
    let command = Cli::parse();
    let client = Connector::new().connect().await?;

    let resolved = client
        .resolve_root(CanonicalPath::canonicalize(command.path())?)
        .await?;

    match command {
        Cli::Del { name, .. } => {
            let result = client.remove_trigger(&resolved, &name).await?;
            println!("{:?}", result);
        }
        Cli::List { .. } => {
            let result = client.list_triggers(&resolved).await?;
            println!("{:?}", result);
        }
        Cli::Register {
            name, output_file, ..
        } => {
            let result = client
                .register_trigger(
                    &resolved,
                    TriggerRequest {
                        name,
                        stdout: Some(output_file),
                        stdin: Some(TriggerStdinConfig::NamePerLine),
                        command: vec!["cat".to_string()],
                        ..Default::default()
                    },
                )
                .await?;
            println!("{:?}", result);
        }
    }

    Ok(())
}
