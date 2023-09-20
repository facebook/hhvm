// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// use std::collections::BTreeMap;
// use std::collections::HashSet;

use hack_macros::hack_stmt;
use naming_special_names_rust::special_idents;
use nast::Afield;
use nast::Block;
use nast::Bop;
use nast::ClassId_;
use nast::Expr;
use nast::Expr_;
use nast::Lid;
use nast::LocalId;
use nast::Pos;
use nast::Stmt;
use nast::Stmt_;
use nast::XhpAttribute;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast::Binop;
use oxidized::nast;

use crate::Env;

#[derive(Debug, Clone, Default)]
struct ContainsAwait {
    found_await: bool,
}

impl ContainsAwait {
    fn check(&mut self, expr: &mut Expr) -> bool {
        self.visit_expr(&mut Env::default(), expr).unwrap();
        self.found_await
    }
}

fn is_await(expr: &Expr) -> bool {
    matches!(expr, Expr((), _pos, Expr_::Await(_)))
}

fn is_assign_await(expr: &Expr) -> bool {
    if let Expr_::Binop(box Binop {
        bop: Bop::Eq(None),
        lhs: _,
        rhs,
    }) = &expr.2
    {
        is_await(rhs)
    } else {
        false
    }
}

impl<'a> VisitorMut<'a> for ContainsAwait {
    type Params = AstParams<Env, ()>;
    fn object(&mut self) -> &mut dyn VisitorMut<'a, Params = Self::Params> {
        self
    }

    fn visit_expr(&mut self, env: &mut Env, elem: &mut nast::Expr) -> Result<(), ()> {
        if matches!(elem.2, Expr_::Efun(_) | Expr_::Lfun(_)) {
            return Ok(());
        };
        if is_await(elem) {
            self.found_await = true;
            Ok(())
        } else {
            elem.recurse(env, self.object())
        }
    }
}

/// Lift await will lift awaits out of expressions into Awaitall statements.
/// It uses tmp_var_counter to allocate temporary variables, and replace_dd
/// to update the $$ variable on the right of |> when it has to sequentialise
/// |> expressions.
#[derive(Debug, Clone, Default)]
struct LiftAwait {
    tmp_var_counter: isize,
    replace_dd: Option<nast::Expr_>,
}

fn sequentialise(pos: Pos, con: Vec<(Lid, nast::Expr)>, seq: Vec<nast::Stmt>) -> Vec<nast::Stmt> {
    if con.is_empty() {
        seq
    } else {
        let ids = Some(con.iter().map(|(lid, _)| lid.clone()).collect());
        let awaitall = Stmt_::Awaitall(Box::new((con, Block(seq))));
        let block = Stmt_::Block(Box::new((ids, Block(vec![Stmt(pos.clone(), awaitall)]))));
        vec![Stmt(pos, block)]
    }
}

fn sequentialise_stmt(
    pos: &Pos,
    elem: &mut nast::Stmt,
    stmt: Stmt_,
    con: Vec<(Lid, nast::Expr)>,
    mut seq: Vec<nast::Stmt>,
) {
    seq.push(Stmt(pos.clone(), stmt));
    let mut stmts = sequentialise(pos.clone(), con, seq);
    if stmts.len() == 1 {
        *elem = stmts.remove(0)
    } else {
        *elem = Stmt(pos.clone(), Stmt_::Block(Box::new((None, Block(stmts)))))
    }
}

impl LiftAwait {
    fn gen_tmp_local(&mut self) -> LocalId {
        let name = special_idents::TMP_VAR_PREFIX.to_string()
            + "_lift_await"
            + &self.tmp_var_counter.to_string();
        self.tmp_var_counter += 1;
        (0, name)
    }

    fn gen_nontmp_local(&mut self) -> LocalId {
        let name = "$__lift_await".to_string()
            + special_idents::TMP_VAR_PREFIX
            + &self.tmp_var_counter.to_string();
        self.tmp_var_counter += 1;
        (0, name)
    }

    fn do_pipe(
        &mut self,
        e: &mut nast::Expr_,
        pos: &Pos,
        con: &mut Vec<(Lid, nast::Expr)>,
        seq: &mut Vec<nast::Stmt>,
    ) {
        match e {
            Expr_::Pipe(box (_, expr1, expr2)) => {
                let mut con1 = vec![];
                let mut seq1 = vec![];
                self.extract_await(expr1, &mut con1, &mut seq1);
                if !ContainsAwait::default().check(expr2) {
                    con.append(&mut con1);
                    seq.append(&mut seq1)
                } else {
                    let tmp1 = self.gen_nontmp_local();
                    let mut replace = Some(Expr_::Lvar(Box::new(Lid(pos.clone(), tmp1.clone()))));
                    std::mem::swap(&mut replace, &mut self.replace_dd);
                    let mut con2 = vec![];
                    let mut seq2 = vec![];
                    self.extract_await(expr2, &mut con2, &mut seq2);
                    std::mem::swap(&mut replace, &mut self.replace_dd);
                    let mut lhs = Expr((), Pos::NONE, Expr_::Null);
                    let mut rhs = lhs.clone();
                    std::mem::swap(&mut lhs, expr1);
                    std::mem::swap(&mut rhs, expr2);
                    seq1.push(hack_stmt!(pos = pos.clone(), "#{lvar(tmp1)} = #lhs;"));
                    let mut stmts1 = sequentialise(pos.clone(), con1, seq1);
                    let tmp2 = self.gen_nontmp_local();
                    // the rhs might have variables assigned in con2, and those get unset at the
                    // end of the awaitall, so we need to assign the rhs to a temporary that
                    // won't get unset.
                    seq2.push(hack_stmt!(
                        pos = pos.clone(),
                        "#{lvar(clone(tmp2))} = #rhs;"
                    ));
                    let mut stmts2 = sequentialise(pos.clone(), con2, seq2);
                    let mut lvar = Expr_::Lvar(Box::new(Lid(pos.clone(), tmp2)));
                    std::mem::swap(&mut lvar, e);
                    stmts1.append(&mut stmts2);
                    seq.append(&mut stmts1)
                }
            }
            _ => panic!(),
        }
    }

    // extract_await(e, &mut con, &mut seq) transforms e by pulling awaits out into con' and
    // turning |> (with await on the rhs) into seq', such that it should be run as Awaitall (con') {seq'; e}.
    // It then appends con' and seq' to con and seq.
    fn extract_await(
        &mut self,
        expr: &mut nast::Expr,
        con: &mut Vec<(Lid, nast::Expr)>,
        seq: &mut Vec<nast::Stmt>,
    ) {
        let Expr((), pos, e) = expr;
        match e {
            Expr_::Lvar(box Lid(_, id)) if id.1 == special_idents::DOLLAR_DOLLAR => {
            if let Some(replace_expr) = &self.replace_dd {
                *e = replace_expr.clone();
            }
        }
        Expr_::Dollardollar(_) => {
            if let Some(replace_expr) = &self.replace_dd {
                *e = replace_expr.clone();
            }
        }
        // Leaf expressions, can't contain await to be lifted
        Expr_::Null
        | Expr_::This
        | Expr_::True
        | Expr_::False
        | Expr_::Omitted
        | Expr_::Id(_)
        | Expr_::Lvar(_)
        | Expr_::Int(_)
        | Expr_::Float(_)
        | Expr_::String(_)
        | Expr_::Efun(_)
        | Expr_::Lfun(_)
        | Expr_::Lplaceholder(_)
        | Expr_::MethodCaller(_)
        | Expr_::Invalid(box None)
        | Expr_::EnumClassLabel(_)
        | Expr_::ClassConst(_)
        | Expr_::FunctionPointer(_)
        | Expr_::ClassGet(_) // Can't contain an expression in strict mode
        | Expr_::Package(_) => {}

        // Expressions with exactly one sub-expression that we can lift an await out of
        Expr_::Invalid(box Some(expr))
        | Expr_::Clone(box expr)
        | Expr_::PrefixedString(box (_, expr))
        | Expr_::ReadonlyExpr(box expr)
        | Expr_::Cast(box (_, expr))
        | Expr_::Is(box (expr, _))
        | Expr_::As(box (expr, _, _))
        | Expr_::Upcast(box (expr, _))
        | Expr_::Import(box (_, expr))
        | Expr_::Hole(box (expr, _, _, _))
        | Expr_::Yield(box Afield::AFvalue(expr))
        | Expr_::ETSplice(box expr)
        | Expr_::Unop(box (_, expr)) => {
            self.extract_await(expr, con, seq)
        }

        // Expressions with exactly two sub-expressions that we can lift an await out of
        // and run concurrently
        Expr_::ArrayGet(box (expr1, Some(expr2)))
        | Expr_::Pair(box (_, expr1, expr2))
        | Expr_::ObjGet(box (expr1, expr2, _, _))
        | Expr_::Yield(box Afield::AFkvalue(expr1, expr2))
        | Expr_::Binop(box nast::Binop{bop:_, lhs:expr1, rhs:expr2}) => {
            // If the operator is || or &&, there are syntactic restrictions that
            // there is no await on the rhs, and so it is safe to traverse it here
            // just to replace $$
            self.extract_await(expr1, con, seq);
            self.extract_await(expr2, con, seq)
        }

        // Expressions with lists of sub-expressions that we can lift an await out of
        // and run concurrently
        Expr_::KeyValCollection(box (_, _, args)) => {
            for nast::Field(arg_k, arg_v) in args {
                self.extract_await(arg_k, con, seq);
                self.extract_await(arg_v, con, seq)
            }
        }
        Expr_::Darray(box (_, args)) => {
            for (arg_k, arg_v) in args {
                self.extract_await(arg_k, con, seq);
                self.extract_await(arg_v, con, seq)
            }
        }
        Expr_::Tuple(args)
        | Expr_::String2(args)
        | Expr_::ValCollection(box(_, _, args))
        | Expr_::Varray(box (_, args)) => {
            for arg in args {
                self.extract_await(arg, con, seq)
            }
        }
        Expr_::Shape(fields) => {
            for (_, expr) in fields {
                self.extract_await(expr, con, seq)
            }
        }
        Expr_::Call(box nast::CallExpr { func, targs: _, args, unpacked_arg }) => {
            self.extract_await(func, con, seq);
            for (_, arg) in args {
                self.extract_await(arg, con, seq)
            }
            for unpack in unpacked_arg {
                self.extract_await(unpack, con, seq)
            }
        }
        Expr_::New(box (nast::ClassId(_, _, cid), _, args, unpacked_arg, _)) => {
            match cid {
                ClassId_::CIexpr(expr) => {
                    self.extract_await(expr, con, seq)
                }
                ClassId_::CIparent
                | ClassId_::CIself
                | ClassId_::CIstatic
                | ClassId_::CI(_) => {}
            }
            for arg in args {
                self.extract_await(arg, con, seq)
            }
            for unpack in unpacked_arg {
                self.extract_await(unpack, con, seq)
            }
        }
        Expr_::Xml(box (_, attrs, exprs)) => {
            for attr in attrs {
                match attr {
                XhpAttribute::XhpSpread(expr)
                | XhpAttribute::XhpSimple(nast::XhpSimple{expr, ..}) =>
                    self.extract_await(expr, con, seq),
                }
            }
            for expr in exprs {
                self.extract_await(expr, con, seq);
            }
        }
        Expr_::Collection(box (_, _, afields)) => {
            for afield in afields {
                match afield {
                    Afield::AFvalue(expr) =>{
                        self.extract_await(expr, con, seq);
                    }
                    Afield::AFkvalue(expr1, expr2) => {
                        self.extract_await(expr1, con, seq);
                        self.extract_await(expr2, con, seq)
                    }
                }
            }
        }

        // Await
        Expr_::Await(box expr1) => {
            let mut awaited_expr = Expr((), Pos::NONE, Expr_::Null);
            std::mem::swap(&mut awaited_expr, expr1);
            if self.replace_dd.is_some() {
                // There's no await in an await (syntactic restriction), so we just
                // need to get the $$ replacement.
                self.extract_await(&mut awaited_expr, con, seq);
            }
            let tmp = Lid(pos.clone(), self.gen_tmp_local());
            let lvar = Expr_::Lvar(Box::new(tmp.clone()));
            *e = lvar;
            con.push((tmp, awaited_expr))
        }
        // Seq
        Expr_::Pipe(box(_, _, _)) => {
          self.do_pipe(e, pos, con, seq);
        }
        Expr_::Eif(box (cond, t, f)) => {
            self.extract_await(cond, con, seq);
            if self.replace_dd.is_some() {
                // Relying on syntactic checks that there are no await in t or f,
                // but still need to traverse in case there is $$
                for expr in t {
                    self.extract_await(expr, con, seq)
                }
                self.extract_await(f, con, seq)
            }
        }

        Expr_::ExpressionTree(box nast::ExpressionTree{ hint:_, splices, function_pointers:_, virtualized_expr:_, runtime_expr, dollardollar_pos:_ }) => {
            for stmt in splices {
                if let Stmt(_, Stmt_::Expr(box Expr((), _, Expr_::Binop(box nast::Binop{ bop:Bop::Eq(None), lhs:_, rhs:expr})))) = stmt {
                    self.extract_await(expr, con, seq)
                }
            }
            self.extract_await(runtime_expr, con, seq)
        }

        // lvalues: shouldn't contain await or $$
        Expr_::ArrayGet(box (_, None))
        | Expr_::List(_) => {}
        }
    }
}

impl<'a> VisitorMut<'a> for LiftAwait {
    type Params = AstParams<Env, ()>;
    fn object(&mut self) -> &mut dyn VisitorMut<'a, Params = Self::Params> {
        self
    }

    fn visit_stmt(&mut self, env: &mut Env, elem: &mut nast::Stmt) -> Result<(), ()> {
        let Stmt(pos, mut stmt_) = std::mem::replace(elem, Stmt(Pos::NONE, Stmt_::Noop));
        let mut con = vec![];
        let mut seq = vec![];
        match &mut stmt_ {
            Stmt_::Expr(expr) if is_assign_await(expr) => {
                expr.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, vec![], vec![]);
                Ok(())
            }
            Stmt_::Expr(box expr) | Stmt_::Return(box Some(expr)) => {
                expr.accept(env, self.object())?;
                if !is_await(expr) {
                    self.extract_await(expr, &mut con, &mut seq);
                }
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::Throw(box expr) => {
                expr.accept(env, self.object())?;
                self.extract_await(expr, &mut con, &mut seq);
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::Return(box None) => {
                sequentialise_stmt(&pos, elem, stmt_, vec![], vec![]);
                Ok(())
            }
            Stmt_::Using(box nast::UsingStmt {
                is_block_scoped: _,
                has_await: _,
                exprs: (_, exprs),
                block,
            }) => {
                for expr in exprs {
                    expr.accept(env, self.object())?;
                    if !is_await(expr) && !is_assign_await(expr) {
                        self.extract_await(expr, &mut con, &mut seq);
                    }
                }
                block.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::If(box (expr, b1, b2)) => {
                expr.accept(env, self.object())?;
                self.extract_await(expr, &mut con, &mut seq);
                b1.accept(env, self.object())?;
                b2.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::For(box (init_exprs, test, update, block)) => {
                for expr in init_exprs {
                    expr.accept(env, self.object())?;
                    self.extract_await(expr, &mut con, &mut seq);
                }
                // No await in the test or update positions (parse error)
                test.accept(env, self.object())?;
                update.accept(env, self.object())?;
                block.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::Switch(box (case, block, default)) => {
                case.accept(env, self.object())?;
                self.extract_await(case, &mut con, &mut seq);
                block.accept(env, self.object())?;
                default.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::Foreach(box (expr, as_expr, block)) => {
                expr.accept(env, self.object())?;
                self.extract_await(expr, &mut con, &mut seq);
                as_expr.accept(env, self.object())?;
                block.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq);
                Ok(())
            }
            Stmt_::DeclareLocal(_) => todo!(), // elaborate typed locals before lifting awaits
            Stmt_::Block(_)
            // await cannot appear in expression in do or while (parse error)
            | Stmt_::Do(_)
            | Stmt_::While(_)
            | Stmt_::Try(_)
            | Stmt_::Match(_)
            | Stmt_::AssertEnv(_)
            | Stmt_::Awaitall(_) => {
                *elem = Stmt(pos, stmt_);
                elem.recurse(env, self.object())
            }
            Stmt_::Noop
            | Stmt_::Fallthrough
            | Stmt_::Break
            | Stmt_::Continue
            | Stmt_::YieldBreak
            | Stmt_::Markup(_) => {
                *elem = Stmt(pos, stmt_);
                Ok(())
            }
        }
    }
}

pub fn elaborate_program(env: &mut Env, program: &mut nast::Program) {
    let mut tl: LiftAwait = Default::default();
    tl.visit_program(env, program).unwrap();
}

// pub fn elaborate_fun_def(env: &mut Env, f: &mut nast::FunDef) {
//     let mut tl: LiftAwait = Default::default();
//     tl.visit_fun_def(env, f).unwrap();
// }

// pub fn elaborate_class_(env: &mut Env, c: &mut nast::Class_) {
//     let mut tl: LiftAwait = Default::default();
//     tl.visit_class_(env, c).unwrap();
// }

#[cfg(test)]
mod tests {
    use hack_macros::hack_expr;
    use hack_macros::hack_stmts;
    use nast::Def;
    use nast::Program;

    use super::*;

    fn build_program(stmt: Stmt) -> Program {
        nast::Program(vec![Def::Stmt(Box::new(stmt))])
    }

    fn mk_lid(name: &str) -> Lid {
        Lid(Pos::NONE, (0, name.to_string()))
    }

    fn mk_lvar(lid: Lid) -> Expr {
        Expr((), Pos::NONE, Expr_::Lvar(Box::new(lid)))
    }

    fn mk_awaitall(con: Vec<(Lid, Expr)>, body: Block) -> Stmt {
        Stmt(
            Pos::NONE,
            Stmt_::Block(Box::new((
                Some(con.iter().map(|(id, _)| id.clone()).collect()),
                Block(vec![Stmt(
                    Pos::NONE,
                    Stmt_::Awaitall(Box::new((con, body))),
                )]),
            ))),
        )
    }

    #[test]
    fn no_await1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("$x0 * $x1 + $x2 * $x3;"));
        let res = build_program(hack_stmt!("$x0 * $x1 + $x2 * $x3;"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn no_await2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("($x0 * $x1) |> ($x2 * $$);"));
        let res = build_program(hack_stmt!("($x0 * $x1) |> ($x2 * $$);"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn no_await3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("($x0 |> $$) + ($x2 |> $$);"));
        let res = build_program(hack_stmt!("($x0 |> $$) + ($x2 |> $$);"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_just_await1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x;"));
        let res = build_program(hack_stmt!("await $x;"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_just_await2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("$y = await $x;"));
        let res = build_program(hack_stmt!("$y = await $x;"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_con1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x + $y;"));
        let tmp = (0, "__tmp$_lift_await0".to_string());
        let awaitall = mk_awaitall(
            vec![(Lid(Pos::NONE, tmp.clone()), hack_expr!("$x"))],
            Block(hack_stmts!("#{lvar(tmp)} + $y;")),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_con2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x0 * await $x1 + await $x2 * await $x3;"));
        let tmp0 = (0, "__tmp$_lift_await0".to_string());
        let tmp1 = (0, "__tmp$_lift_await1".to_string());
        let tmp2 = (0, "__tmp$_lift_await2".to_string());
        let tmp3 = (0, "__tmp$_lift_await3".to_string());
        let awaitall = mk_awaitall(
            vec![
                (Lid(Pos::NONE, tmp0.clone()), hack_expr!("$x0")),
                (Lid(Pos::NONE, tmp1.clone()), hack_expr!("$x1")),
                (Lid(Pos::NONE, tmp2.clone()), hack_expr!("$x2")),
                (Lid(Pos::NONE, tmp3.clone()), hack_expr!("$x3")),
            ],
            Block(hack_stmts!(
                "#{lvar(tmp0)} * #{lvar(tmp1)} + #{lvar(tmp2)} * #{lvar(tmp3)};"
            )),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_left1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x |> $$;"));
        let tmp = (0, "__tmp$_lift_await0".to_string());
        let awaitall = mk_awaitall(
            vec![(Lid(Pos::NONE, tmp.clone()), hack_expr!("$x"))],
            Block(hack_stmts!("#{lvar(tmp)} |> $$;")),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_left2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("(await $x1 + await $x2) |> $$;"));
        let tmp0 = (0, "__tmp$_lift_await0".to_string());
        let tmp1 = (0, "__tmp$_lift_await1".to_string());
        let awaitall = mk_awaitall(
            vec![
                (Lid(Pos::NONE, tmp0.clone()), hack_expr!("$x1")),
                (Lid(Pos::NONE, tmp1.clone()), hack_expr!("$x2")),
            ],
            Block(hack_stmts!("#{lvar(tmp0)} + #{lvar(tmp1)} |> $$;")),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_right() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("$x |> await $$;"));
        let tmp0 = mk_lvar(mk_lid("$__lift_await__tmp$0"));
        let tmp1 = mk_lid("__tmp$_lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lvar(mk_lid("$__lift_await__tmp$2"));
        let awaitall = mk_awaitall(
            vec![(tmp1, tmp0.clone())],
            Block(hack_stmts!("#{clone(tmp2)} = #tmp1_lvar;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp2)));
        let res = build_program(hack_stmt!("{ #tmp0 = $x; #awaitall; #stmt; }"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_both() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x |> await $$;"));
        let tmp0 = mk_lid("__tmp$_lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lvar(mk_lid("$__lift_await__tmp$1"));
        let tmp2 = mk_lid("__tmp$_lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lvar(mk_lid("$__lift_await__tmp$3"));
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#{clone(tmp1)} = #tmp0_lvar;")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1)],
            Block(hack_stmts!("#{clone(tmp3)} = #tmp2_lvar;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp3)));
        let res = build_program(hack_stmt!("{ #awaitall1; #awaitall2; #stmt; }"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_both2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!(
            "(await $x + 1 |> await $$ + 2) |> await $$ + 3;"
        ));
        let tmp0 = mk_lid("__tmp$_lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lvar(mk_lid("$__lift_await__tmp$1"));
        let tmp2 = mk_lid("__tmp$_lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lvar(mk_lid("$__lift_await__tmp$3"));
        let tmp4 = mk_lvar(mk_lid("$__lift_await__tmp$4"));
        let tmp5 = mk_lid("__tmp$_lift_await5");
        let tmp5_lvar = mk_lvar(tmp5.clone());
        let tmp6 = mk_lvar(mk_lid("$__lift_await__tmp$6"));
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#{clone(tmp1)} = #tmp0_lvar + 1;")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1)],
            Block(hack_stmts!("#{clone(tmp3)} = #tmp2_lvar + 2;")),
        );
        let awaitall3 = mk_awaitall(
            vec![(tmp5, tmp4.clone())],
            Block(hack_stmts!("#{clone(tmp6)} = #tmp5_lvar + 3;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp6)));
        let res = build_program(hack_stmt!(
            "{ #awaitall1; #awaitall2; #tmp4 = #tmp3; #awaitall3; #stmt; }"
        ));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_both3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!(
            "await $x + 1 |> (await $$ + 2 |> await $$ + 3);"
        ));
        let tmp0 = mk_lid("__tmp$_lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lvar(mk_lid("$__lift_await__tmp$1"));
        let tmp2 = mk_lid("__tmp$_lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lvar(mk_lid("$__lift_await__tmp$3"));
        let tmp4 = mk_lid("__tmp$_lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let tmp5 = mk_lvar(mk_lid("$__lift_await__tmp$5"));
        let tmp6 = mk_lvar(mk_lid("$__lift_await__tmp$6"));
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#{clone(tmp1)} = #{clone(tmp0_lvar)} + 1;")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1)],
            Block(hack_stmts!("#{clone(tmp3)} = #{clone(tmp2_lvar)} + 2;")),
        );
        let awaitall3 = mk_awaitall(
            vec![(tmp4, tmp3)],
            Block(hack_stmts!("#{clone(tmp5)} = #{clone(tmp4_lvar)} + 3;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp6.clone())));
        let res = build_program(hack_stmt!(
            "{ #awaitall1; #awaitall2; #awaitall3; #tmp6 = #tmp5; #stmt; }"
        ));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_of_con() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!(
            "(await $x + await $y) |> (await $$ + await $$);"
        ));
        let tmp0 = mk_lid("__tmp$_lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$_lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lvar(mk_lid("$__lift_await__tmp$2"));
        let tmp3 = mk_lid("__tmp$_lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lid("__tmp$_lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let tmp5 = mk_lvar(mk_lid("$__lift_await__tmp$5"));
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x")), (tmp1, hack_expr!("$y"))],
            Block(hack_stmts!(
                "#{clone(tmp2)} = #{clone(tmp0_lvar)} + #{clone(tmp1_lvar)};"
            )),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp3, tmp2.clone()), (tmp4, tmp2)],
            Block(hack_stmts!("#{clone(tmp5)} = #tmp3_lvar + #tmp4_lvar;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp5)));
        let res = build_program(hack_stmt!("{ #awaitall1; #awaitall2; #stmt; }"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_con_of_seq() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x + (await $y |> await $$);"));
        let tmp0 = mk_lid("__tmp$_lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$_lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lvar(mk_lid("$__lift_await__tmp$2"));
        let tmp3 = mk_lid("__tmp$_lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lvar(mk_lid("$__lift_await__tmp$4"));
        let awaitall1 = mk_awaitall(
            vec![(tmp1, hack_expr!("$y"))],
            Block(hack_stmts!("#{clone(tmp2)} = #{clone(tmp1_lvar)};")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp3, tmp2)],
            Block(hack_stmts!("#{clone(tmp4)} = #{clone(tmp3_lvar)};")),
        );
        let _stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp4.clone())));
        let awaitall3 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#awaitall1; #awaitall2; #tmp0_lvar + #tmp4;")),
        );
        let res = build_program(hack_stmt!("#awaitall3;"));
        self::elaborate_program(&mut env, &mut orig);
        assert_eq!(orig, res);
    }
}
