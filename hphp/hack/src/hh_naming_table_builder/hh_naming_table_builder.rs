// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Generates a naming table saved state and outputs it to the given file

fn main() -> anyhow::Result<()> {
    std::env::set_var("RUST_BACKTRACE", "1");
    std::env::set_var("RUST_LIB_BACKTRACE", "1");
    if matches!(std::env::var("RUST_LOG").ok().as_deref(), None | Some("")) {
        std::env::set_var("RUST_LOG", "INFO");
    }

    let args = <naming_table_builder::Args as clap::Parser>::parse();
    let status = naming_table_builder::build_naming_table(args)?;
    std::process::exit(status.as_code());
}
