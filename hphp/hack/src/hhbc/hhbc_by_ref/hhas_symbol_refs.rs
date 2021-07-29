// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use hhbc_by_ref_symbol_refs_state::{IncludePath, IncludePathSet, SSet};

/// Data structure for keeping track of symbols (and includes) we encounter in
///the course of emitting bytecode for an AST. We split them into these four
/// categories for the sake of HHVM, which has lookup function corresponding to each.
#[derive(Clone, Debug, Default)]
pub struct HhasSymbolRefs<'arena> {
    pub includes: IncludePathSet<'arena>,
    pub constants: SSet<'arena>,
    pub functions: SSet<'arena>,
    pub classes: SSet<'arena>,
}
