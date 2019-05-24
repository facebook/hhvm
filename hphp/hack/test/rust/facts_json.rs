// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.;

use std::fs;

extern crate clap;
use clap::{App, Arg};

use facts_rust::facts_parser::*;

fn main() {
    let args = App::new("Facts JSON test (Rust)")
        .arg(
            Arg::with_name("file-path")
                .long("file-path")
                .takes_value(true)
                .required(true),
        )
        .arg(Arg::with_name("force-hh").long("force-hh"))
        .arg(Arg::with_name("enable-xhp").long("enable-xhp"))
        .get_matches();

    let file_path = args.value_of("file-path").unwrap_or("").to_owned();
    let opts = ExtractAsJsonOpts {
        php5_compat_mode: true,
        hhvm_compat_mode: true,
        force_hh: args.is_present("force-hh"),
        enable_xhp: args.is_present("enable-xhp"),
        filename: file_path.clone(),
    };

    let content = fs::read(&file_path).expect("failed to read file");
    let content_str = &String::from_utf8_lossy(&content);
    let json = extract_as_json(content_str, opts).unwrap_or("{}".to_owned());
    println!("{}", json);
}
