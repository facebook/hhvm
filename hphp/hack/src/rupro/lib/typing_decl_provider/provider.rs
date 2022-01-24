// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use crate::folded_decl_provider::FoldedDeclProvider;
use crate::pos::Symbol;
use crate::reason::Reason;
use crate::typing_decl_provider::{Class, TypingDeclCache};

#[derive(Debug)]
pub struct TypingDeclProvider<R: Reason> {
    cache: Rc<dyn TypingDeclCache<Reason = R>>,
    folded_decl_provider: Rc<FoldedDeclProvider<R>>,
}

impl<R: Reason> TypingDeclProvider<R> {
    pub fn new(
        cache: Rc<dyn TypingDeclCache<Reason = R>>,
        folded_decl_provider: Rc<FoldedDeclProvider<R>>,
    ) -> Self {
        Self {
            cache,
            folded_decl_provider,
        }
    }

    pub fn get_folded_decl_provider(&self) -> &Rc<FoldedDeclProvider<R>> {
        &self.folded_decl_provider
    }

    pub fn get_class(&self, name: &Symbol) -> Option<Rc<Class<R>>> {
        match self.cache.get_typing_class(name) {
            Some(rc) => Some(rc),
            None => {
                let folded_decl = self.folded_decl_provider.get_folded_class(name)?;
                let cls = Rc::new(Class::new(folded_decl));
                self.cache.put_typing_class(name.clone(), cls.clone());
                Some(cls)
            }
        }
    }

    pub fn get_class_or_typedef(&self, name: &Symbol) -> Option<Rc<Class<R>>> {
        self.get_class(name)
    }
}
