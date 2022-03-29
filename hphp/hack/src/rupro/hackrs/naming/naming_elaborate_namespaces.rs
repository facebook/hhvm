// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::special_names as sn;
use oxidized::aast_visitor::{Params, VisitorMut};
use oxidized::{aast::*, ast_defs::*};
use std::collections::HashSet;
use utils::core::ns;

struct ElaborateNamespacesVisitor<'node> {
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
    fn new() -> Self {
        Self {
            type_params: HashSet::new(),
        }
    }


    fn is_reserved_type_hint(&self, name: &str) -> bool {
        let name = ns::strip_ns(name);
        sn::typehints::reserved_typehints
            .iter()
            .any(|s| s.as_str() == name)
    }

    fn elaborate_type_name(&self, id: &mut Id) {
        rupro_todo_mark!(Naming, "special identifiers, $, namespaces");
        if !self.type_params.contains(id.1.as_str()) {
            if id.1.chars().next().map_or(false, |c| c != '\\') {
                id.1 = ns::add_ns(&id.1);
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
        rupro_todo_mark!(Naming, "xhp, codegen");
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

pub fn elaborate_class(cls: &mut Class_<(), ()>) {
    let mut vis = ElaborateNamespacesVisitor::new();
    vis.visit_class_(&mut (), cls).unwrap();
}
