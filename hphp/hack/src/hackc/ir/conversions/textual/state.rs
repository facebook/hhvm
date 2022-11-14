// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::sync::Arc;

use hash::HashMap;
use ir::StringInterner;

use crate::textual;

#[derive(Eq, PartialEq)]
pub(crate) enum FuncDeclKind {
    Internal,
    External,
}

pub(crate) struct UnitState {
    pub(crate) decls: Decls,
    pub(crate) strings: Arc<StringInterner>,
}

impl UnitState {
    pub(crate) fn new(strings: Arc<StringInterner>) -> Self {
        Self {
            decls: Default::default(),
            strings,
        }
    }
}

#[derive(Default)]
pub(crate) struct Decls {
    pub(crate) funcs: HashMap<String, FuncDeclKind>,
    pub(crate) globals: HashMap<String, textual::Ty>,
}

impl Decls {
    pub(crate) fn declare_func<'a>(&mut self, name: impl Into<Cow<'a, str>>, kind: FuncDeclKind) {
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

    pub(crate) fn declare_global<'a>(&mut self, name: impl Into<Cow<'a, str>>, ty: textual::Ty) {
        let name = name.into().into_owned();
        self.globals.entry(name).or_insert(ty);
    }

    pub(crate) fn merge(&mut self, other: Decls) {
        for (name, kind) in other.funcs {
            self.declare_func(name, kind)
        }
        for (name, ty) in other.globals {
            self.declare_global(name, ty)
        }
    }

    pub(crate) fn external_funcs(&self) -> impl Iterator<Item = &str> {
        self.funcs.iter().filter_map(|(name, kind)| match kind {
            FuncDeclKind::Internal => None,
            FuncDeclKind::External => Some(name.as_str()),
        })
    }
}
