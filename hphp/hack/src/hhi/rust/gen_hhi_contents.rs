// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::path::PathBuf;

use clap::Parser;

#[derive(Debug, Parser)]
struct Options {
    /// The directory containing HHI files.
    #[clap(long)]
    hhi_dir: PathBuf,

    /// The directory containing this stamp is the directory to search for HHIs
    /// generated from the HSL. These will be placed in the final hhi directory
    /// under a subdirectory named "hsl_generated".
    #[clap(long)]
    hsl_stamp: PathBuf,
}

fn main() {
    // This is the entrypoint when used from buck.
    let opts = Options::parse();
    let out_dir = std::env::var("OUT").unwrap(); // $OUT implicitly provided by buck
    run(opts, &out_dir)
}

fn run(opts: Options, out_dir: &str) {
    let hsl_dir = opts.hsl_stamp.parent().unwrap();

    let mut hhi_contents = vec![];

    hhi_contents.extend(get_hhis_in_dir(&opts.hhi_dir));
    hhi_contents.extend(
        get_hhis_in_dir(hsl_dir)
            .map(|(path, contents)| (PathBuf::from("hsl_generated").join(path), contents)),
    );

    let out_filename = PathBuf::from(out_dir).join("lib.rs");
    gen_hhi_contents_lib::write_hhi_contents_file(&out_filename, &hhi_contents).unwrap();
}

fn get_hhis_in_dir(root: &Path) -> impl Iterator<Item = (PathBuf, String)> + '_ {
    walkdir::WalkDir::new(root)
        .sort_by_file_name()
        .into_iter()
        .map(|e| e.unwrap())
        .filter(|e| e.file_type().is_file())
        .filter(|e| e.path().extension().and_then(|s| s.to_str()) == Some("hhi"))
        .map(move |e| {
            let contents = std::fs::read_to_string(e.path()).unwrap();
            let relative_path = e.path().strip_prefix(root).unwrap().to_owned();
            (relative_path, contents)
        })
}
