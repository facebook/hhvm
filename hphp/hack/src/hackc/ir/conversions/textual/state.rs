// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::sync::Arc;

use hash::HashMap;
use ir::StringInterner;

#[derive(Eq, PartialEq)]
pub(crate) enum FuncDeclKind {
    Internal,
    External,
}

pub(crate) struct UnitState {
    pub(crate) func_declares: FuncDecls,
    pub(crate) strings: Arc<StringInterner>,
}

impl UnitState {
    pub(crate) fn new(strings: Arc<StringInterner>) -> Self {
        Self {
            func_declares: Default::default(),
            strings,
        }
    }
}

#[derive(Default)]
pub(crate) struct FuncDecls {
    pub(crate) funcs: HashMap<String, FuncDeclKind>,
}

impl FuncDecls {
    pub(crate) fn declare<'a>(&mut self, name: impl Into<Cow<'a, str>>, kind: FuncDeclKind) {
        let name = name.into().into_owned();
        match kind {
            FuncDeclKind::Internal => {
                // We're declaring an internal function definition. If there was
                // no prior definition then add it. If the prior definition was
                // external then upgrade it to internal.
                self.funcs
                    .entry(name)
                    .and_modify(|v| *v = FuncDeclKind::Internal)
                    .or_insert(FuncDeclKind::Internal);
            }
            FuncDeclKind::External => {
                // We're declaring an external function reference. If we already
                // know about this function (either external or internal) just
                // keep that.
                self.funcs.entry(name).or_insert(FuncDeclKind::External);
            }
        }
    }

    pub(crate) fn merge(&mut self, other: FuncDecls) {
        for (name, kind) in other.funcs {
            self.declare(name, kind)
        }
    }

    pub(crate) fn external_funcs(&self) -> impl Iterator<Item = &str> {
        self.funcs.iter().filter_map(|(name, kind)| match kind {
            FuncDeclKind::Internal => None,
            FuncDeclKind::External => Some(name.as_str()),
        })
    }
}
