// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path;
use structopt::StructOpt;
use cbindgen::*;

#[derive(Debug, structopt::StructOpt)]
#[structopt(
    name = "ffi_cbindgen",
    about = "
Generate a cbindgen style C++ header for a list of .rs sources.

Example invocation:

  ffi_cbindgen \
--header hhbc-ast.h \
--srcs \
hphp/hack/src/utils/ffi/ffi.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.rs \
--namespaces HPHP,hackc,hhbc
")]
struct Opt {
    // Input files
    #[structopt(long="srcs", parse(from_os_str), default_value="", use_delimiter = true)]
    srcs: Vec<path::PathBuf>,

    // The header file to write
    #[structopt(long="header", parse(from_os_str))]
    header: path::PathBuf,

    // Any namespaces to wrap the generated code in
    #[structopt(long="namespaces", default_value="", use_delimiter = true)]
    namespaces: Vec<String>,
}

fn mk_builder(opts: &Opt) -> Builder {
    let config: Config = Config {
        language: Language::Cxx,
        macro_expansion: MacroExpansionConfig { bitflags: true },
        ..Default::default()
    };
    const LICENSE: &str = "\
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>";
    let mut builder =
        Builder::new()
        .with_config(config)
        .with_header(LICENSE)
        .with_pragma_once(true)
        ;
    for src in opts.srcs.iter() {
        builder = builder.with_src(src)
    }
    match &opts.namespaces[..] {
        [namespace, namespaces @ ..] =>
            builder
            .with_namespace(namespace)
            .with_namespaces(namespaces),
        [] => builder,
    }
}

fn main () {
    let opts = Opt::from_args();
    let header: &path::PathBuf = &opts.header;
    mk_builder(&opts)
        .generate()
        .expect(format!("[ffi_cbindgen] Generation of \"{}\" failed.", header.display()).as_ref())
        .write_to_file(header)
        ;
}
