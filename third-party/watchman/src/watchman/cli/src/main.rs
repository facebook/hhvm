/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use structopt::clap::AppSettings;
use structopt::StructOpt;

mod audit;

#[derive(StructOpt, Debug)]
#[structopt(setting = AppSettings::DisableVersion,
    setting = AppSettings::VersionlessSubcommands)]
struct MainCommand {
    #[structopt(subcommand)]
    subcommand: TopLevelSubcommand,
}

#[derive(StructOpt, Debug)]
enum TopLevelSubcommand {
    Audit(audit::AuditCmd),
}

impl TopLevelSubcommand {
    async fn run(&self) -> anyhow::Result<()> {
        use TopLevelSubcommand::*;
        match self {
            Audit(cmd) => cmd.run().await,
        }
    }
}

#[tokio::main]
async fn main() {
    let cmd = MainCommand::from_args();
    match cmd.subcommand.run().await {
        Ok(()) => {}
        Err(e) => {
            eprintln!("error: {}", e);
            std::process::exit(1);
        }
    }
}
