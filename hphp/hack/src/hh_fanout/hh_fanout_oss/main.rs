use std::io;
use std::path::PathBuf;

use clap::Parser;

/// Executable wrapper around hh_fanout functionality
/// that provides saved state support for HHVM open-source.
///
/// Builds a 64-bit dependency graph from a collection of edges.
#[derive(Parser)]
#[command(about)]
struct Args {
    /// A file containing a dependency graph delta in binary format,
    /// as produced by `hh --save-state /path/`.
    #[arg(long)]
    delta_file: PathBuf,

    /// Where to put the 64-bit dependency graph.
    #[arg(long)]
    output: PathBuf,
}

fn main() -> io::Result<()> {
    let args = Args::parse();

    hh_fanout_build::build(
        false,
        None,
        None,
        Some(args.delta_file.into_os_string()),
        &args.output,
    )
}
