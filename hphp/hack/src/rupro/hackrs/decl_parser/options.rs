// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref::decl_parser_options::DeclParserOptions;
use std::fmt;

/// Owning wrapper for `oxidized_by_ref::decl_parser_options::DeclParserOptions`
/// which is `Clone + Debug + Send + Sync`.
pub struct Options(ArenaAndOpts);

#[ouroboros::self_referencing]
struct ArenaAndOpts {
    arena: Box<bumpalo::Bump>,
    #[borrows(arena)]
    #[covariant]
    opts: DeclParserOptions<'this>,
}

// Safe because the arena cannot be accessed once the ArenaAndOpts is
// constructed (because none of the generated ouroboros methods, or any other
// way to access the arena, are exposed by this module).
unsafe impl Sync for ArenaAndOpts {}

impl Options {
    pub fn get(&self) -> &DeclParserOptions<'_> {
        self.0.borrow_opts()
    }
}

impl From<&DeclParserOptions<'_>> for Options {
    fn from(opts: &DeclParserOptions<'_>) -> Self {
        Self(
            ArenaAndOptsBuilder {
                arena: Box::new(bumpalo::Bump::new()),
                opts_builder: |arena| opts.clone_in(arena),
            }
            .build(),
        )
    }
}

impl Default for Options {
    fn default() -> Self {
        Self(
            ArenaAndOptsBuilder {
                arena: Box::new(bumpalo::Bump::new()),
                opts_builder: |_arena| Default::default(),
            }
            .build(),
        )
    }
}

impl Clone for Options {
    fn clone(&self) -> Self {
        Self::from(self.get())
    }
}

impl fmt::Debug for Options {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.get().fmt(f)
    }
}
