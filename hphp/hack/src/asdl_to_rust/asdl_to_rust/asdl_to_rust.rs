// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Generate Rust datatypes for abstract syntax defined by ASDL.
#[derive(Debug, clap::Parser)]
struct Opts {
    src: std::path::PathBuf,
}

fn parse_opts() -> Result<Opts, std::io::Error> {
    let opts = <Opts as clap::Parser>::parse();
    let src = &opts.src;
    std::fs::metadata(src)?;
    Ok(opts)
}

fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync + 'static>> {
    let opts = parse_opts()?;

    let lexerdef = asdl::lexer::lexerdef();
    let asdl = std::fs::read_to_string(&opts.src)?;
    let lexer = lexerdef.lexer(&asdl);
    let (res, errs) = asdl::parser::parse(&lexer);
    for e in errs {
        println!("{}", e.pp(&lexer, &asdl::parser::token_epp));
    }
    match res {
        Some(r) => println!("Result: {:#?}", r),
        _ => eprintln!("Unable to evaluate expression."),
    }
    Ok(())
}
