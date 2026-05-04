/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use clap::Parser;

mod audit;
mod rage;

#[derive(Parser, Debug)]
#[command(disable_version_flag = true)]
struct MainCommand {
    #[command(subcommand)]
    subcommand: TopLevelSubcommand,
}

#[derive(Parser, Debug)]
enum TopLevelSubcommand {
    Audit(audit::AuditCmd),
    Rage(rage::RageCmd),
}

impl TopLevelSubcommand {
    async fn run(&self) -> anyhow::Result<()> {
        use TopLevelSubcommand::*;
        match self {
            Audit(cmd) => cmd.run().await,
            Rage(cmd) => cmd.run().await,
        }
    }
}

#[tokio::main]
async fn main() {
    let cmd = MainCommand::parse();
    match cmd.subcommand.run().await {
        Ok(()) => {}
        Err(e) => {
            eprintln!("error: {}", e);
            std::process::exit(1);
        }
    }
}
