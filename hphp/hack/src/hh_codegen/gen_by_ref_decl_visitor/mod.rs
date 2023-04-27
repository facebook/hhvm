// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod args;
mod node_impl_generator;
mod visitor_trait_generator;

pub use args::Args;

use crate::common::context::Context;

pub fn run(args: &Args) -> anyhow::Result<Vec<(std::path::PathBuf, String)>> {
    let files = crate::common::parse_all(&args.common.input)?;
    let extern_files = crate::common::parse_all(&args.extern_input)?;

    let ctx =
        Context::with_extern_files(files.as_slice(), extern_files.as_slice(), &args.common.root)?;

    let results = vec![
        ("node.rs", crate::common::by_ref_node::node()),
        ("node_impl.rs", crate::common::by_ref_node::node_impl()),
        ("node_impl_gen.rs", node_impl_generator::gen(&ctx)),
        ("visitor.rs", visitor_trait_generator::gen(&ctx)),
    ];
    Ok(results
        .iter()
        .map(|(filename, source)| (args.common.output.join(filename), source.to_string()))
        .collect())
}
