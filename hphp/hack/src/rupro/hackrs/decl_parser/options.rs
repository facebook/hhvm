// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref::parser_options::ParserOptions;
use std::fmt;

/// Owning wrapper for `oxidized_by_ref::global_options::GlobalOptions` which is
/// `Clone + Debug + Send + Sync`.
pub struct Options(ArenaAndOpts);

#[ouroboros::self_referencing]
struct ArenaAndOpts {
    arena: Box<bumpalo::Bump>,
    #[borrows(arena)]
    #[covariant]
    opts: ParserOptions<'this>,
}

// Safe because the arena cannot be accessed once the ArenaAndOpts is
// constructed (because none of the generated ouroboros methods, or any other
// way to access the arena, are exposed by this module).
unsafe impl Sync for ArenaAndOpts {}

impl Options {
    pub fn get(&self) -> &ParserOptions<'_> {
        self.0.borrow_opts()
    }
}

impl From<&ParserOptions<'_>> for Options {
    fn from(opts: &ParserOptions<'_>) -> Self {
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
        Self::from(ParserOptions::DEFAULT)
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
