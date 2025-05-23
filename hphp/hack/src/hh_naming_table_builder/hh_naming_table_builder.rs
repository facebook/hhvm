// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Generates a naming table saved state and outputs it to the given file

fn main() -> anyhow::Result<()> {
    // TODO: Audit that the environment access only happens in single-threaded code.
    unsafe { std::env::set_var("RUST_BACKTRACE", "1") };
    // TODO: Audit that the environment access only happens in single-threaded code.
    unsafe { std::env::set_var("RUST_LIB_BACKTRACE", "1") };
    if matches!(std::env::var("RUST_LOG").ok().as_deref(), None | Some("")) {
        // TODO: Audit that the environment access only happens in single-threaded code.
        unsafe { std::env::set_var("RUST_LOG", "INFO") };
    }

    let args = <naming_table_builder::Args as clap::Parser>::parse();
    let status = naming_table_builder::build_naming_table(args)?;
    std::process::exit(status.as_code());
}
