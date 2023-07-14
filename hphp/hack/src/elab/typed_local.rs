// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;
use std::collections::HashSet;

#[allow(unused_imports)]
use hack_macros::hack_stmts;
use nast::Binop;
use nast::Expr;
use nast::Expr_;
use nast::Hint;
use nast::Hint_;
use nast::Lid;
use nast::Pos;
use nast::Stmt;
use nast::Stmt_;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast_defs::Bop;
use oxidized::local_id;
use oxidized::naming_error::NamingError;
use oxidized::nast;

use crate::Env;

#[derive(Debug, Clone, Default)]
pub struct TypedLocal {
    // The locals in scope. The hint is present iff this is a typed local.
    // Union at join points to keep the locals that might be in scope.
    // The position is where the local was first declared.
    locals: BTreeMap<String, (Option<Hint>, Pos)>,
    // The subset of typed locals declared in the current scope, rather than preceding it.
    declared_ids: HashSet<String>,
    // The subset of untyped locals first assigned in the current scope, rather than preceding it.
    assigned_ids: HashSet<String>,
    should_elab: bool,
}

impl TypedLocal {
    fn add_local(&mut self, id: String, hint: Option<Hint>, pos: &Pos) {
        self.locals.insert(id, (hint, pos.clone()));
    }

    fn add_assigned_id(&mut self, id: String) {
        self.assigned_ids.insert(id);
    }
    fn add_declared_id(&mut self, id: &str) {
        self.declared_ids.insert(id.to_string());
    }

    fn get_local(&self, id: &str) -> Option<&(Option<Hint>, Pos)> {
        self.locals.get(id)
    }

    fn get_local_pos(&self, id: &str) -> Pos {
        if let Some((_, pos)) = self.get_local(id) {
            pos.clone()
        } else {
            Pos::NONE
        }
    }

    fn clear(&mut self) {
        self.assigned_ids.clear();
        self.declared_ids.clear();
        self.locals.clear();
    }

    fn join(
        &mut self,
        other: Self,
        env: &mut Env,
        mut declared_above: HashSet<String>,
        mut assigned_above: HashSet<String>,
    ) {
        for id in self.declared_ids.intersection(&other.declared_ids) {
            let id_pos = other.get_local_pos(id);
            let def_pos = self.get_local_pos(id);
            env.emit_error(NamingError::IllegalTypedLocal {
                join: true,
                id_pos,
                id_name: id.clone(),
                def_pos,
            });
        }
        for id in self.declared_ids.intersection(&other.assigned_ids) {
            let assign_pos = other.get_local_pos(id);
            let decl_pos = self.get_local_pos(id);
            env.emit_error(NamingError::IllegalTypedLocal {
                join: true,
                id_pos: decl_pos,
                id_name: id.clone(),
                def_pos: assign_pos,
            });
        }
        for id in self.assigned_ids.intersection(&other.declared_ids) {
            let assign_pos = self.get_local_pos(id);
            let decl_pos = other.get_local_pos(id);
            env.emit_error(NamingError::IllegalTypedLocal {
                join: true,
                id_pos: decl_pos,
                id_name: id.clone(),
                def_pos: assign_pos,
            });
        }

        // self.locals contains the cumulative bindings down the self branch.
        // For the bindings newly defined in other branch, insert them into
        // locals if they aren't already there
        for id in other.declared_ids.iter().chain(other.assigned_ids.iter()) {
            if !self.locals.contains_key(id) {
                if let Some(x) = other.get_local(id) {
                    self.locals.insert(id.to_string(), x.clone());
                }
            }
        }
        std::mem::swap(&mut self.declared_ids, &mut declared_above);
        self.declared_ids.extend(
            declared_above
                .into_iter()
                .chain(other.declared_ids.into_iter()),
        );
        std::mem::swap(&mut self.assigned_ids, &mut assigned_above);
        self.assigned_ids.extend(
            assigned_above
                .into_iter()
                .chain(other.assigned_ids.into_iter()),
        );
    }

    fn get_hint(&mut self, expr: &Expr) -> Option<Hint> {
        match expr {
            Expr(_, pos, Expr_::Lvar(box lid)) => {
                let name = local_id::get_name(&lid.1);
                if let Some((hint_opt, _pos)) = self.get_local(name) {
                    hint_opt.clone()
                } else {
                    self.add_local(name.to_string(), None, pos);
                    self.add_assigned_id(name.to_string());
                    None
                }
            }
            // TODO, Unclear how to do enforcement with an as on the rhs
            Expr(_, _pos, Expr_::List(..)) => None,
            // TODO, What if we are enforcing a shape type that we don't know the definition of?
            Expr(_, _pos, Expr_::ArrayGet(..)) => None,
            _ => None,
        }
    }

    fn wrap_rhs_with_as(&self, rhs: &mut Expr, hint: Option<Hint>, pos: &Pos) {
        if self.should_elab && let Some(hint) = hint {
            let mut init_expr = Expr((), Pos::NONE, Expr_::Null);
            std::mem::swap(rhs, &mut init_expr);
            let as_expr_ = Expr_::As(Box::new((init_expr, hint, false)));
            let as_expr = Expr((), pos.clone(), as_expr_);
            *rhs = as_expr;
        }
    }

    fn enforce_assign_expr(&mut self, expr: &mut Expr) {
        match expr {
            Expr(
                (),
                pos,
                Expr_::Binop(box Binop {
                    bop: Bop::Eq(_), // TODO, check all possibilities
                    lhs,
                    rhs,
                }),
            ) => {
                let hint = self.get_hint(lhs);
                self.wrap_rhs_with_as(rhs, hint, pos);
            }
            _ => {}
        }
    }
}

impl<'a> VisitorMut<'a> for TypedLocal {
    type Params = AstParams<Env, ()>;
    fn object(&mut self) -> &mut dyn VisitorMut<'a, Params = Self::Params> {
        self
    }

    fn visit_stmt(&mut self, env: &mut Env, elem: &mut nast::Stmt) -> Result<(), ()> {
        let Stmt(pos, stmt_) = elem;
        match stmt_ {
            Stmt_::DeclareLocal(box (lid, hint, expr)) => {
                let name = local_id::get_name(&lid.1);
                if let Some((_, def_pos)) = self.get_local(name) {
                    env.emit_error(NamingError::IllegalTypedLocal {
                        join: false,
                        id_pos: pos.clone(),
                        id_name: name.clone(),
                        def_pos: def_pos.clone(),
                    });
                } else {
                    self.add_local(name.to_string(), Some(hint.clone()), pos);
                    self.add_declared_id(name);
                    if self.should_elab && let Some(expr) = expr {
                        let mut as_hint = Hint(Pos::NONE, Box::new(Hint_::Hnothing));
                        std::mem::swap(hint, &mut as_hint);
                        self.wrap_rhs_with_as(expr, Some(as_hint), pos);
                        let mut init_lid = Lid(Pos::NONE, (0, "".to_string()));
                        std::mem::swap(&mut init_lid, lid);
                        let mut init_expr = Expr((), Pos::NONE, Expr_::Null);
                        std::mem::swap(expr, &mut init_expr);
                        let assign_expr_ = Expr_::Binop(Box::new(Binop {
                            bop: Bop::Eq(None),
                            lhs: Expr((), pos.clone(), Expr_::Lvar(Box::new(init_lid))),
                            rhs: init_expr,
                        }));
                        let assign_expr = Expr((), pos.clone(), assign_expr_);
                        *stmt_ = Stmt_::Expr(Box::new(assign_expr));
                    } else if self.should_elab {
                        *stmt_ = Stmt_::Noop;
                    }
                };
                Ok(())
            }
            Stmt_::Expr(box expr) => {
                self.enforce_assign_expr(expr);
                Ok(())
            }
            Stmt_::If(box (_cond, then_block, else_block)) => {
                let mut declared_above = HashSet::default();
                let mut assigned_above = HashSet::default();
                std::mem::swap(&mut self.declared_ids, &mut declared_above);
                std::mem::swap(&mut self.assigned_ids, &mut assigned_above);
                let mut alt = self.clone();
                then_block.recurse(env, self.object())?;
                else_block.recurse(env, alt.object())?;
                self.join(alt, env, declared_above, assigned_above);
                Ok(())
            }
            Stmt_::For(box (init_exprs, _, update_exprs, body)) => {
                for init_expr in init_exprs.iter_mut() {
                    self.enforce_assign_expr(init_expr);
                }
                body.recurse(env, self.object())?;
                for update_expr in update_exprs.iter_mut() {
                    self.enforce_assign_expr(update_expr);
                }
                Ok(())
            }
            Stmt_::Using(_) => elem.recurse(env, self.object()), // TODO
            Stmt_::Switch(_) => elem.recurse(env, self.object()), // TODO
            Stmt_::Foreach(_) => elem.recurse(env, self.object()), // TODO
            Stmt_::Try(_) => elem.recurse(env, self.object()),   // TODO
            // Just need to visit these, no additional logic is required
            Stmt_::Fallthrough
            | Stmt_::Awaitall(_)
            | Stmt_::Break
            | Stmt_::Continue
            | Stmt_::Throw(_)
            | Stmt_::Return(_)
            | Stmt_::YieldBreak
            | Stmt_::Do(_)
            | Stmt_::While(_)
            | Stmt_::Noop
            | Stmt_::Block(_)
            | Stmt_::Markup(_)
            | Stmt_::AssertEnv(_) => elem.recurse(env, self.object()),
            // TODO: check lambdas properly
        }
    }

    fn visit_fun_def(&mut self, env: &mut Env, elem: &mut nast::FunDef) -> Result<(), ()> {
        self.clear();
        for param in elem.fun.params.iter() {
            self.add_local(param.name.clone(), None, &param.pos);
            self.add_assigned_id(param.name.clone());
        }
        elem.fun.body.fb_ast.recurse(env, self.object())?;
        Ok(())
    }

    fn visit_method_(&mut self, env: &mut Env, elem: &mut nast::Method_) -> Result<(), ()> {
        self.clear();
        for param in elem.params.iter() {
            self.add_local(param.name.clone(), None, &param.pos);
            self.add_assigned_id(param.name.clone());
        }
        elem.body.fb_ast.recurse(env, self.object())?;
        Ok(())
    }
}

pub fn elaborate_program(env: &mut Env, program: &mut nast::Program, should_elab: bool) {
    let mut tl = TypedLocal {
        should_elab,
        ..Default::default()
    };
    tl.visit_program(env, program).unwrap();
}

pub fn elaborate_fun_def(env: &mut Env, f: &mut nast::FunDef, should_elab: bool) {
    let mut tl = TypedLocal {
        should_elab,
        ..Default::default()
    };
    tl.visit_fun_def(env, f).unwrap();
}

pub fn elaborate_class_(env: &mut Env, c: &mut nast::Class_, should_elab: bool) {
    let mut tl = TypedLocal {
        should_elab,
        ..Default::default()
    };
    tl.visit_class_(env, c).unwrap();
}

#[cfg(test)]
mod tests {

    use nast::Block;
    use nast::Def;
    use nast::Program;

    use super::*;

    fn build_program(stmts: Vec<Stmt>) -> Program {
        nast::Program(vec![Def::Stmt(Box::new(Stmt(
            Pos::NONE,
            Stmt_::Block(Block(stmts)),
        )))])
    }

    #[test]
    fn test_init() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: int = 1;"));
        let res = build_program(hack_stmts!("$x = 1 as int;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_init2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: int;"));
        let res = build_program(hack_stmts!(""));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: vec<int> = 1; $x = vec[];"));
        let res = build_program(hack_stmts!("$x = 1 as vec<int>; $x = vec[] as vec<int>;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: vec<int>; $x = vec[];"));
        let noop = Stmt(Pos::NONE, Stmt_::Noop);
        let res = build_program(hack_stmts!("#noop; $x = vec[] as vec<int>;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_if1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x: vec<string> = 1; if ($b) {$x = vec[];} else {$x = 1;}; $x = 4;"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as vec<string>; if ($b) {$x = vec[] as vec<string>;} else {$x = 1 as vec<string>;}; $x = 4 as vec<string>;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_if2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "if ($b) {let $x: t = vec[]; $x = 1;} else {let $y: t2 = vec[]; $y = 1;}; $x = 4; $y = 44;"
        ));
        let res = build_program(hack_stmts!(
            "if ($b) {$x = vec[] as t; $x = 1 as t;} else {$y = vec[] as t2; $y = 1 as t2;}; $x = 4 as t; $y = 44 as t2;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_for1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x: t = 1; let $y: t2 = 2; for ($x = 1, $y = 2; ; $x = 1, $y = 2) { $x = 1; }; $y = 3;"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as t; $y = 2 as t2; for ($x = 1 as t, $y = 2 as t2; ; $x= 1 as t, $y = 2 as t2) { $x = 1 as t; }; $y = 3 as t2;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_for2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "for (; ; $x = 1) { let $x: t = 1; $x = 22;} $x = 3;"
        ));
        let res = build_program(hack_stmts!(
            "for (; ; $x = 1 as t) { $x = 1 as t; $x = 22 as t;} $x = 3 as t;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }
}
