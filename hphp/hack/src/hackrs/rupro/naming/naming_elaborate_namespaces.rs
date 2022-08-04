// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use im::HashSet;
use oxidized::aast::*;
use oxidized::aast_visitor::Params;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast;
use oxidized::ast_defs::*;
use special_names as sn;
use utils::core::ns;

struct ElaborateNamespacesVisitor<'node> {
    type_params: &'node HashSet<String>,
}

struct ElaborateNamespacesVisitorParams;

impl Params for ElaborateNamespacesVisitorParams {
    type Context = ();
    type Error = ();
    type Ex = ();
    type En = ();
}

impl<'node> ElaborateNamespacesVisitor<'node> {
    fn new(type_params: &'node HashSet<String>) -> Self {
        Self { type_params }
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

    fn visit_expr_(
        &mut self,
        _c: &mut <Self::Params as Params>::Context,
        p: &'node mut Expr_<(), ()>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        rupro_todo_mark!(AST);
        use Expr_::*;
        match p {
            Call(box (Expr(_ty, _pos, Id(box ast::Id(_p, id))), _targs, _el, _uarg)) => {
                rupro_todo_mark!(Naming, "NS.elaborate_id");
                *id = ns::add_ns(id);
            }
            _ => {}
        };
        Ok(())
    }
}

pub fn elaborate_class(type_params: &HashSet<String>, cls: &mut Class_<(), ()>) {
    let mut vis = ElaborateNamespacesVisitor::new(type_params);
    vis.visit_class_(&mut (), cls).unwrap();
}

pub fn elaborate_fun_def(type_params: &HashSet<String>, fd: &mut FunDef<(), ()>) {
    let mut vis = ElaborateNamespacesVisitor::new(type_params);
    vis.visit_fun_def(&mut (), fd).unwrap();
}
