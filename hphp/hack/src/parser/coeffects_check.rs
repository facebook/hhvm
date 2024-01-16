// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use core_utils_rust as utils;
use naming_special_names_rust::coeffects;
use naming_special_names_rust::coeffects::Ctx;
use naming_special_names_rust::members;
use naming_special_names_rust::special_idents;
use naming_special_names_rust::user_attributes;
use oxidized::aast;
use oxidized::aast::Binop;
use oxidized::aast::Hint_;
use oxidized::aast_defs;
use oxidized::aast_defs::Hint;
use oxidized::aast_visitor::visit;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::aast_visitor::Visitor;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::local_id;
use oxidized::pos::Pos;
use parser_core_types::syntax_error;
use parser_core_types::syntax_error::Error as ErrorMsg;
use parser_core_types::syntax_error::SyntaxError;

fn hints_contain_capability(hints: &[aast_defs::Hint], capability: Ctx) -> bool {
    for hint in hints {
        if let aast::Hint_::Happly(ast_defs::Id(_, id), _) = &*hint.1 {
            let (_, name) = utils::split_ns_from_name(id);
            let c = coeffects::ctx_str_to_enum(name);
            match c {
                Some(val) => {
                    if coeffects::capability_matches_name(&capability, &val) {
                        return true;
                    }
                }
                None => continue,
            }
        }
    }
    false
}

fn has_capability(ctxs: Option<&ast::Contexts>, capability: Ctx) -> bool {
    match ctxs {
        Some(c) => hints_contain_capability(&c.1, capability),
        // No capabilities context is the same as defaults in scenarios where contexts are not inherited
        None => match capability {
            Ctx::WriteProps | Ctx::WriteThisProps | Ctx::ReadGlobals | Ctx::Globals => true,
            _ => false,
        },
    }
}

fn is_mutating_unop(unop: &ast_defs::Uop) -> bool {
    use ast_defs::Uop::*;
    match unop {
        Upincr | Uincr | Updecr | Udecr => true,
        _ => false,
    }
}

// This distinguishes between contexts which explicitly do not have write_props and unspecified contexts which may implicitly lack write_props.
// The latter is important because for e.g. a lambda without contexts, it should inherit context from its enclosing function.
fn has_capability_with_inherited_val(
    c: &Context,
    ctxs: Option<&ast::Contexts>,
    capability: Ctx,
) -> bool {
    match ctxs {
        Some(_) => has_capability(ctxs, capability),
        None => match capability {
            Ctx::WriteProps => c.has_write_props(),
            Ctx::WriteThisProps => c.has_write_this_props(),
            Ctx::ReadGlobals => c.has_read_globals(),
            Ctx::Globals => c.has_access_globals(),
            _ => false,
        },
    }
}

fn has_defaults_with_inherited_val(c: &Context, ctxs: Option<&ast::Contexts>) -> bool {
    match ctxs {
        Some(_) => has_defaults(ctxs),
        None => c.has_defaults(),
    }
}

fn has_defaults_implicitly(hints: &[Hint]) -> bool {
    for hint in hints {
        let Hint(_, h) = hint;
        match &**h {
            Hint_::Happly(ast_defs::Id(_, id), _) => {
                let (_, name) = utils::split_ns_from_name(id);
                match coeffects::ctx_str_to_enum(name) {
                    Some(c) => match c {
                        Ctx::Defaults => {
                            return true;
                        }
                        _ => {
                            continue;
                        }
                    },
                    None => {
                        return true;
                    }
                }
            }
            aast::Hint_::HfunContext(_) => {
                continue;
            }
            aast::Hint_::Haccess(Hint(_, hint), sids) => match &**hint {
                Hint_::Happly(ast_defs::Id(..), _) if !sids.is_empty() => {
                    continue;
                }
                Hint_::Hvar(_) if sids.len() == 1 => {
                    continue;
                }
                _ => {
                    return true;
                }
            },
            _ => {
                return true;
            }
        }
    }
    false
}

fn has_defaults(ctxs: Option<&ast::Contexts>) -> bool {
    match ctxs {
        None => true,
        Some(c) => {
            if c.1.is_empty() {
                false
            } else {
                has_defaults_implicitly(&c.1)
            }
        }
    }
}

fn is_constructor_or_clone(s: &ast_defs::Id) -> bool {
    let ast_defs::Id(_, name) = s;
    name == members::__CONSTRUCT || name == members::__CLONE
}

fn has_ignore_coeffect_local_errors_attr(attrs: &[aast::UserAttribute<(), ()>]) -> bool {
    for attr in attrs.iter() {
        let ast_defs::Id(_, name) = &attr.name;
        if user_attributes::ignore_coeffect_local_errors(name) {
            return true;
        }
    }
    false
}

bitflags! {
    #[derive(PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Clone, Copy)]
    pub struct ContextFlags: u16 {
        /// Hack, very roughly, has three contexts in which you can see an
        /// expression:
        /// - Expression Trees
        /// - Initializer positions
        /// - Everything else
        /// There are other rules that handle expr trees and initializer
        /// positions, so we really care about all the other contexts. This is
        /// mostly inside function bodies, but also includes the right-hand-side
        /// of an enum class initializer, which can contain function calls and
        /// other complex expressions.
        const IN_COMPLEX_EXPRESSION_CONTEXT = 1 << 0;
        const IS_TYPECHECKER = 1 << 1;
        const IS_CONSTRUCTOR_OR_CLONE = 1 << 2;
        const IGNORE_COEFFECT_LOCAL_ERRORS = 1 << 3;
        const HAS_DEFAULTS = 1 << 4;
        const HAS_WRITE_PROPS = 1 << 5;
        const HAS_WRITE_THIS_PROPS = 1 << 6;
        const HAS_READ_GLOBALS = 1 << 7;
        const HAS_ACCESS_GLOBALS = 1 << 8;
    }
}

struct Context {
    bitflags: ContextFlags,
}

impl Context {
    fn new() -> Self {
        Self {
            bitflags: ContextFlags::from_bits_truncate(0_u16),
        }
    }

    fn from_context(&self) -> Self {
        let mut c = Context::new();
        c.set_in_complex_expression_context(self.in_complex_expression_context());
        c.set_is_constructor_or_clone(self.is_constructor_or_clone());
        c.set_ignore_coeffect_local_errors(self.ignore_coeffect_local_errors());
        c.set_is_typechecker(self.is_typechecker());
        c.set_has_defaults(self.has_defaults());
        c.set_has_write_props(self.has_write_props());
        c.set_has_write_this_props(self.has_write_this_props());
        c
    }

    fn in_complex_expression_context(&self) -> bool {
        self.bitflags
            .contains(ContextFlags::IN_COMPLEX_EXPRESSION_CONTEXT)
    }

    fn is_constructor_or_clone(&self) -> bool {
        self.bitflags
            .contains(ContextFlags::IS_CONSTRUCTOR_OR_CLONE)
    }

    fn ignore_coeffect_local_errors(&self) -> bool {
        self.bitflags
            .contains(ContextFlags::IGNORE_COEFFECT_LOCAL_ERRORS)
    }

    fn has_defaults(&self) -> bool {
        self.bitflags.contains(ContextFlags::HAS_DEFAULTS)
    }

    fn is_typechecker(&self) -> bool {
        self.bitflags.contains(ContextFlags::IS_TYPECHECKER)
    }

    fn has_write_props(&self) -> bool {
        self.bitflags.contains(ContextFlags::HAS_WRITE_PROPS)
    }

    fn has_write_this_props(&self) -> bool {
        self.bitflags.contains(ContextFlags::HAS_WRITE_THIS_PROPS)
    }

    fn has_read_globals(&self) -> bool {
        self.bitflags.contains(ContextFlags::HAS_READ_GLOBALS)
    }

    fn has_access_globals(&self) -> bool {
        self.bitflags.contains(ContextFlags::HAS_ACCESS_GLOBALS)
    }

    fn set_in_complex_expression_context(&mut self, in_complex_expression_context: bool) {
        self.bitflags.set(
            ContextFlags::IN_COMPLEX_EXPRESSION_CONTEXT,
            in_complex_expression_context,
        );
    }

    fn set_is_constructor_or_clone(&mut self, is_constructor_or_clone: bool) {
        self.bitflags.set(
            ContextFlags::IS_CONSTRUCTOR_OR_CLONE,
            is_constructor_or_clone,
        );
    }

    fn set_ignore_coeffect_local_errors(&mut self, ignore_coeffect_local_errors: bool) {
        self.bitflags.set(
            ContextFlags::IGNORE_COEFFECT_LOCAL_ERRORS,
            ignore_coeffect_local_errors,
        );
    }

    fn set_has_defaults(&mut self, has_defaults: bool) {
        self.bitflags.set(ContextFlags::HAS_DEFAULTS, has_defaults);
    }

    fn set_is_typechecker(&mut self, is_typechecker: bool) {
        self.bitflags
            .set(ContextFlags::IS_TYPECHECKER, is_typechecker);
    }

    fn set_has_write_props(&mut self, has_write_props: bool) {
        self.bitflags
            .set(ContextFlags::HAS_WRITE_PROPS, has_write_props);
    }

    fn set_has_write_this_props(&mut self, has_write_this_props: bool) {
        self.bitflags
            .set(ContextFlags::HAS_WRITE_THIS_PROPS, has_write_this_props);
    }

    fn set_has_read_globals(&mut self, has_read_globals: bool) {
        self.bitflags
            .set(ContextFlags::HAS_READ_GLOBALS, has_read_globals);
    }

    fn set_has_access_globals(&mut self, has_access_globals: bool) {
        self.bitflags
            .set(ContextFlags::HAS_ACCESS_GLOBALS, has_access_globals);
    }
}

struct Checker {
    errors: Vec<SyntaxError>,
}

impl Checker {
    fn new() -> Self {
        Self { errors: vec![] }
    }

    fn add_error(&mut self, pos: &Pos, msg: ErrorMsg) {
        let (start_offset, end_offset) = pos.info_raw();
        self.errors
            .push(SyntaxError::make(start_offset, end_offset, msg, vec![]));
    }

    fn do_write_props_check(&mut self, e: &aast::Expr<(), ()>) {
        if let Some(Binop {
            bop: ast_defs::Bop::Eq(_),
            lhs,
            ..
        }) = e.2.as_binop()
        {
            self.check_is_obj_property_write_expr(e, lhs, true /* ignore_this_writes */);
        }
        if let Some((unop, expr)) = e.2.as_unop() {
            if is_mutating_unop(unop) {
                self.check_is_obj_property_write_expr(e, expr, true /* ignore_this_writes */);
            }
        }
    }

    fn do_write_this_props_check(&mut self, c: &mut Context, e: &aast::Expr<(), ()>) {
        if c.is_constructor_or_clone() {
            return;
        }
        if let Some(aast::Binop {
            bop: ast_defs::Bop::Eq(_),
            lhs,
            ..
        }) = e.2.as_binop()
        {
            self.check_is_obj_property_write_expr(e, lhs, false /* ignore_this_writes */);
        }
        if let Some((unop, expr)) = e.2.as_unop() {
            if is_mutating_unop(unop) {
                self.check_is_obj_property_write_expr(e, expr, false /* ignore_this_writes */);
            }
        }
    }

    fn check_is_obj_property_write_expr(
        &mut self,
        top_level_expr: &aast::Expr<(), ()>,
        expr: &aast::Expr<(), ()>,
        ignore_this_writes: bool,
    ) {
        if let Some((lhs, ..)) = expr.2.as_obj_get() {
            if let Some(v) = lhs.2.as_lvar() {
                if local_id::get_name(&v.1) == special_idents::THIS {
                    if ignore_this_writes {
                        return;
                    } else {
                        self.add_error(
                            &top_level_expr.1,
                            syntax_error::write_props_without_capability,
                        );
                    }
                }
            }
            self.add_error(
                &top_level_expr.1,
                syntax_error::write_props_without_capability,
            );
        } else if let Some((arr, _)) = expr.2.as_array_get() {
            self.check_is_obj_property_write_expr(top_level_expr, arr, ignore_this_writes);
        }
    }
}

impl<'ast> Visitor<'ast> for Checker {
    type Params = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
        self
    }

    fn visit_class_(&mut self, c: &mut Context, cls: &aast::Class_<(), ()>) -> Result<(), ()> {
        let mut new_context = Context::from_context(c);
        if cls.enum_.is_some() {
            // If we're in an enum class, we need to ensure that the following
            // is illegal:
            //
            //   enum class Foo: _ {
            //     _ BAR = Baz::$bing;
            //   }
            //
            new_context.set_in_complex_expression_context(true);
        }
        cls.recurse(&mut new_context, self)
    }

    fn visit_method_(&mut self, c: &mut Context, m: &aast::Method_<(), ()>) -> Result<(), ()> {
        let mut new_context = Context::from_context(c);
        new_context.set_in_complex_expression_context(true);
        new_context.set_is_constructor_or_clone(is_constructor_or_clone(&m.name));
        new_context.set_ignore_coeffect_local_errors(has_ignore_coeffect_local_errors_attr(
            &m.user_attributes,
        ));
        let ctxs = m.ctxs.as_ref();
        new_context.set_has_defaults(has_defaults(ctxs));
        new_context.set_has_write_props(has_capability(ctxs, Ctx::WriteProps));
        new_context.set_has_write_this_props(has_capability(ctxs, Ctx::WriteThisProps));
        new_context.set_has_read_globals(has_capability(ctxs, Ctx::ReadGlobals));
        new_context.set_has_access_globals(has_capability(ctxs, Ctx::Globals));
        m.recurse(&mut new_context, self)
    }

    fn visit_fun_def(&mut self, c: &mut Context, d: &aast::FunDef<(), ()>) -> Result<(), ()> {
        let mut new_context = Context::from_context(c);
        let ctxs = d.fun.ctxs.as_ref();
        new_context.set_has_defaults(has_defaults(ctxs));
        new_context.set_is_constructor_or_clone(is_constructor_or_clone(&d.name));
        new_context.set_has_write_props(has_capability(ctxs, Ctx::WriteProps));
        new_context.set_has_write_this_props(has_capability(ctxs, Ctx::WriteThisProps));
        new_context.set_has_read_globals(has_capability(ctxs, Ctx::ReadGlobals));
        new_context.set_has_access_globals(has_capability(ctxs, Ctx::Globals));
        d.recurse(&mut new_context, self)
    }

    fn visit_fun_(&mut self, c: &mut Context, f: &aast::Fun_<(), ()>) -> Result<(), ()> {
        let mut new_context = Context::from_context(c);
        new_context.set_in_complex_expression_context(true);
        new_context.set_ignore_coeffect_local_errors(has_ignore_coeffect_local_errors_attr(
            &f.user_attributes,
        ));
        let ctxs = f.ctxs.as_ref();
        new_context.set_has_defaults(has_defaults_with_inherited_val(c, ctxs));
        new_context.set_has_write_props(has_capability_with_inherited_val(
            c,
            ctxs,
            Ctx::WriteProps,
        ));
        new_context.set_has_write_this_props(has_capability_with_inherited_val(
            c,
            ctxs,
            Ctx::WriteThisProps,
        ));
        new_context.set_has_read_globals(has_capability_with_inherited_val(
            c,
            ctxs,
            Ctx::ReadGlobals,
        ));
        new_context.set_has_access_globals(has_capability_with_inherited_val(
            c,
            ctxs,
            Ctx::Globals,
        ));
        f.recurse(&mut new_context, self)
    }

    fn visit_expr(&mut self, c: &mut Context, p: &aast::Expr<(), ()>) -> Result<(), ()> {
        if c.in_complex_expression_context()
            && !c.ignore_coeffect_local_errors()
            && !c.has_defaults()
        {
            if !c.has_write_props() {
                self.do_write_props_check(p);
                if !c.has_write_this_props() {
                    self.do_write_this_props_check(c, p);
                }
            }
            if !c.has_access_globals() {
                // Do access globals check e.g. C::$x = 5;
                if let Some(Binop {
                    bop: ast_defs::Bop::Eq(_),
                    lhs,
                    rhs,
                }) = p.2.as_binop()
                {
                    if let Some((_, _, ast_defs::PropOrMethod::IsProp)) = lhs.2.as_class_get() {
                        self.add_error(&lhs.1, syntax_error::access_globals_without_capability);
                        // Skipping one level of recursion when read_globals are present
                        // ensures the left hand side global is not subject to read_global
                        // enforcement, thus duplicating the error message.
                        if c.has_read_globals() {
                            return rhs.recurse(c, self);
                        }
                    }
                }
                // Skipping one level of recursion for a readonly expression when
                // read_globals are present ensures globals enclosed in read_globals
                // are not subject to enforcement.
                if let Some(e) = p.2.as_readonly_expr() {
                    if c.has_read_globals() {
                        // Do not subject the readonly static itself to read_globals check,
                        // only its potential children
                        return e.recurse(c, self);
                    }
                }
                if let Some((_, _, ast_defs::PropOrMethod::IsProp)) = p.2.as_class_get() {
                    if !c.has_read_globals() {
                        self.add_error(&p.1, syntax_error::read_globals_without_capability)
                    } else {
                        self.add_error(&p.1, syntax_error::read_globals_without_readonly)
                    }
                }
            }
        }
        p.recurse(c, self)
    }
}

pub fn check_program(program: &aast::Program<(), ()>, is_typechecker: bool) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context::new();
    context.set_is_typechecker(is_typechecker);
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
