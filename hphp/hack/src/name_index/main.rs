// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use names::Names;
use std::path::PathBuf;
use structopt::StructOpt;

/// CommandLineArgs declaratively defines the command-line interface of recli.
/// Structopt is responsible for deriving a command-line parser from this
/// structure.
#[derive(Debug, StructOpt)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
#[structopt(rename_all = "kebab")] // rename every option to kebab-case
struct CommandLineArgs {
    /// Root path to use
    #[structopt(short, long, parse(from_os_str))]
    root: PathBuf,

    /// Where to load the naming table from
    #[structopt(short, long, parse(from_os_str))]
    naming_table: PathBuf,

    /// The name of the const
    #[structopt(short, long)]
    gconst: String,

    /// Daemonize and run as a service in the background
    #[structopt(long)]
    daemonize: bool,
}

fn main() {
    let args = CommandLineArgs::from_args();

    let naming_table_path = args.naming_table.as_path();

    let names = Names::readonly_from_file(naming_table_path).unwrap();

    let path = names.get_const_path(&args.gconst).unwrap();

    println!("{:?}", path);
}
