// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::special_names::SpecialNames;
use crate::utils::core::utils;
use oxidized::aast_visitor::{Params, VisitorMut};
use oxidized::{aast::*, ast_defs::*};
use std::collections::HashSet;

pub struct ElaborateNamespaces {
    special_names: &'static SpecialNames,
}

struct ElaborateNamespacesVisitor<'node> {
    special_names: &'static SpecialNames,
    type_params: HashSet<&'node str>,
}

struct ElaborateNamespacesVisitorParams;

impl Params for ElaborateNamespacesVisitorParams {
    type Context = ();
    type Error = ();
    type Ex = ();
    type En = ();
}

impl<'node> ElaborateNamespacesVisitor<'node> {
    fn new(special_names: &'static SpecialNames) -> Self {
        Self {
            special_names,
            type_params: HashSet::new(),
        }
    }


    fn is_reserved_type_hint(&self, name: &str) -> bool {
        let name = utils::strip_ns(name);
        self.special_names
            .typehints
            .reserved_typehints
            .iter()
            .any(|s| s.as_str() == name)
    }

    fn elaborate_type_name(&self, id: &mut Id) {
        // TODO(hrust): special_identifier, $
        if !self.type_params.contains(id.1.as_str()) {
            // TODO(hrust): namespaces
            if id.1.chars().next().map_or(false, |c| c != '\\') {
                id.1 = utils::add_ns(&id.1);
            }
        }
    }
}

impl<'node> VisitorMut<'node> for ElaborateNamespacesVisitor<'node> {
    type Params = ElaborateNamespacesVisitorParams;

    fn object(&mut self) -> &mut dyn VisitorMut<'node, Params = Self::Params> {
        self
    }

    fn visit_hint_(
        &mut self,
        _c: &mut <Self::Params as Params>::Context,
        p: &'node mut Hint_,
    ) -> Result<(), <Self::Params as Params>::Error> {
        // TODO(hrust): is_xhp_screwup, codegen
        match p {
            Hint_::Happly(id, _hl) => {
                if !self.is_reserved_type_hint(&id.1) {
                    self.elaborate_type_name(id);
                }
            }
            _ => {}
        }
        Ok(())
    }
}

impl ElaborateNamespaces {
    pub fn new(special_names: &'static SpecialNames) -> Self {
        Self { special_names }
    }
    pub fn on_class_(&self, cls: &mut Class_<(), ()>) {
        let mut vis = ElaborateNamespacesVisitor::new(self.special_names);
        vis.visit_class_(&mut (), cls).unwrap();
    }
}
