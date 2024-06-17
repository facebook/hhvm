/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::path::PathBuf;

use clap::Parser;
use serde::Deserialize;
use watchman_client::prelude::*;

/// Perform a glob query for a path, using watchman
#[derive(Debug, Parser)]
struct Cli {
    #[arg(default_value = ".")]
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

    println!("resolved watch to {:?}", resolved);

    // Basic globs -> names
    let files = client.glob(&resolved, &["**/*.rs"]).await?;
    println!("files: {:#?}", files);

    query_result_type! {
        struct NameAndHash {
            name: NameField,
            hash: ContentSha1HexField,
        }
    }

    let response: QueryResult<NameAndHash> = client
        .query(
            &resolved,
            QueryRequestCommon {
                glob: Some(vec!["**/*.rs".to_string()]),
                expression: Some(Expr::Not(Box::new(Expr::Empty))),
                ..Default::default()
            },
        )
        .await?;
    println!("response: {:#?}", response);

    Ok(())
}
