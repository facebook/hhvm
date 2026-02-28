use std::path::PathBuf;

use clap::Parser;

#[derive(Parser)]
struct Options {
    input: PathBuf,

    #[clap(short)]
    output: PathBuf,
}

fn main() -> std::io::Result<()> {
    let options = Options::parse();
    decompress::decompress(&options.input, &options.output)
}
