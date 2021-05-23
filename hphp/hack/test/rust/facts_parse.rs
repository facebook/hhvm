// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.;

extern crate clap;

use std::fs;
use std::path::PathBuf;

use clap::{App, Arg};
use facts_rust::facts_parser::*;
use oxidized::relative_path::*;

fn main() {
    let args = App::new("Facts JSON test (Rust)")
        .arg(
            Arg::with_name("file-path")
                .long("file-path")
                .multiple(true)
                .min_values(1)
                .required(true),
        )
        .arg(
            Arg::with_name("parse-only")
                .long("parse-only")
                .help("Parse Facts but don't convert them to JSON"),
        )
        .get_matches();

    for file_path in args.values_of("file-path").unwrap() {
        parse(file_path.to_owned(), args.is_present("parse-only"));
    }
}

fn parse(file_path: String, parse_only: bool) {
    let path = RelativePath::make(Prefix::Dummy, PathBuf::from(file_path.clone()));
    let opts = ExtractAsJsonOpts {
        php5_compat_mode: true,
        hhvm_compat_mode: true,
        allow_new_attribute_syntax: false,
        enable_xhp_class_modifier: false,
        disable_xhp_element_mangling: false,
        disallow_hash_comments: true,
        filename: path,
    };

    let content = fs::read(&file_path).expect("failed to read file");
    if parse_only {
        println!("{}", from_text(&content, opts).is_some());
        return;
    }
    let json = extract_as_json(&content, opts).unwrap_or("{}".to_owned());
    println!("{}", json);
}
