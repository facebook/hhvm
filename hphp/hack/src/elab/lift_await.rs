// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// use std::collections::BTreeMap;
// use std::collections::HashSet;

use hack_macros::hack_expr;
use hack_macros::hack_stmt;
use naming_special_names_rust::special_idents;
use nast::Afield;
use nast::Block;
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
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast;
use oxidized::parsing_error::ParsingError;

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

/// How an expression uses await in its sub-expressions
enum AwaitUsage {
    /// The expression does not await
    NoAwait,
    /// The expression has a single await
    ContainsAwait,
    /// The expression contains multiple awaits that can be run concurrently
    ConcurrentAwait,
    /// The expression contains multiple awaits that must be sequentialised
    Sequential,
    /// The expression contains multiple awaits that require concurrent execution of
    /// sequentialised awaits. For example `await f($x) + (await g($y) |> await h($$))`
    Error(Option<Pos>),
}

fn combine_con(await1: AwaitUsage, await2: AwaitUsage) -> AwaitUsage {
    use AwaitUsage::*;
    match (await1, await2) {
        (Sequential, NoAwait) | (NoAwait, Sequential) => Sequential,
        // Errors propagate
        (Error(Some(pos)), _) => Error(Some(pos)),
        (_, Error(Some(pos))) => Error(Some(pos)),
        (Error(None), _) | (_, Error(None)) => Error(None),
        // Can't concurrently combine a sequential with an await
        (Sequential, _) | (_, Sequential) => Error(None),
        // If both sides have an await, then we have concurrent composition
        (ConcurrentAwait, _) | (_, ConcurrentAwait) | (ContainsAwait, ContainsAwait) => {
            ConcurrentAwait
        }
        (NoAwait, ContainsAwait) | (ContainsAwait, NoAwait) => ContainsAwait,
        (NoAwait, NoAwait) => NoAwait,
    }
}

/// check how the expression uses nested awaits
fn check_await_usage(expr: &Expr) -> AwaitUsage {
    use AwaitUsage::*;
    let Expr((), pos, e) = expr;
    let await_usage = match e {
        // Expressions with no sub-expressions that we can lift an await out of
        Expr_::Dollardollar(_)
        | Expr_::Null
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
        | Expr_::Nameof(_)
        | Expr_::FunctionPointer(_)
        | Expr_::ClassGet(_)
        | Expr_::Package(_) => NoAwait,

        // Expressions with exactly one sub-expression that we can lift an await out of
        Expr_::Invalid(box Some(expr))
        | Expr_::Clone(box expr)
        | Expr_::PrefixedString(box (_, expr))
        | Expr_::ReadonlyExpr(box expr)
        | Expr_::Cast(box (_, expr))
        | Expr_::Is(box (expr, _))
        | Expr_::As(box nast::As_ { expr, .. })
        | Expr_::Upcast(box (expr, _))
        | Expr_::Import(box (_, expr))
        | Expr_::Hole(box (expr, _, _, _))
        | Expr_::Yield(box Afield::AFvalue(expr))
        | Expr_::ETSplice(box nast::EtSplice {
            spliced_expr: expr, ..
        })
        | Expr_::Unop(box (_, expr))
        | Expr_::ArrayGet(box (expr, None)) => check_await_usage(expr),

        // Expressions with exactly two sub-expressions that we can lift an await out of
        // and run concurrently
        Expr_::ArrayGet(box (expr1, Some(expr2)))
        | Expr_::Pair(box (_, expr1, expr2))
        | Expr_::ObjGet(box (expr1, expr2, _, _))
        | Expr_::Yield(box Afield::AFkvalue(expr1, expr2))
        | Expr_::Binop(box nast::Binop {
            bop: _, // Can't have await in || or && (parse error)
            lhs: expr1,
            rhs: expr2,
        })
        | Expr_::Assign(box (expr1, _, expr2)) => {
            combine_con(check_await_usage(expr1), check_await_usage(expr2))
        }

        // Expressions with lists of sub-expressions that we can lift an await out of
        // and run concurrently
        Expr_::KeyValCollection(box (_, _, args)) => args
            .iter()
            .map(|nast::Field(arg_k, arg_v)| {
                combine_con(check_await_usage(arg_k), check_await_usage(arg_v))
            })
            .fold(NoAwait, combine_con),
        Expr_::Tuple(args) | Expr_::String2(args) | Expr_::ValCollection(box (_, _, args)) => args
            .iter()
            .map(check_await_usage)
            .fold(NoAwait, combine_con),
        Expr_::Shape(fields) => fields
            .iter()
            .map(|(_, expr)| check_await_usage(expr))
            .fold(NoAwait, combine_con),
        Expr_::Call(box nast::CallExpr {
            func,
            targs: _,
            args,
            unpacked_arg,
        }) => {
            let res = args
                .iter()
                .map(|arg| check_await_usage(arg.to_expr_ref()))
                .fold(check_await_usage(func), combine_con);
            unpacked_arg
                .iter()
                .map(check_await_usage)
                .fold(res, combine_con)
        }
        Expr_::New(box (nast::ClassId(_, _, cid), _, args, unpacked_arg, _)) => {
            let mut res = match cid {
                ClassId_::CIexpr(expr) => check_await_usage(expr),
                ClassId_::CIparent | ClassId_::CIself | ClassId_::CIstatic | ClassId_::CI(_) => {
                    NoAwait
                }
            };
            res = args.iter().map(check_await_usage).fold(res, combine_con);
            unpacked_arg
                .iter()
                .map(check_await_usage)
                .fold(res, combine_con)
        }
        Expr_::Xml(box (_, attrs, exprs)) => {
            let res = attrs
                .iter()
                .map(|attr| match attr {
                    XhpAttribute::XhpSpread(expr)
                    | XhpAttribute::XhpSimple(nast::XhpSimple { expr, .. }) => {
                        check_await_usage(expr)
                    }
                })
                .fold(NoAwait, combine_con);
            exprs.iter().map(check_await_usage).fold(res, combine_con)
        }
        Expr_::Collection(box (_, _, afields)) => afields
            .iter()
            .map(|afield| match afield {
                Afield::AFvalue(expr) => check_await_usage(expr),
                Afield::AFkvalue(expr1, expr2) => {
                    combine_con(check_await_usage(expr1), check_await_usage(expr2))
                }
            })
            .fold(NoAwait, combine_con),
        // Await
        Expr_::Await(box expr1) => match check_await_usage(expr1) {
            NoAwait => ContainsAwait,
            ContainsAwait | Sequential | ConcurrentAwait => Sequential,
            Error(p) => Error(p),
        },
        // Seq
        Expr_::Pipe(box (_, expr1, expr2)) => match check_await_usage(expr2) {
            Error(p) => Error(p),
            // If the rhs contains an await, then we need to finish the lhs
            // prior to doing the await
            ContainsAwait | ConcurrentAwait | Sequential => Sequential,
            NoAwait => check_await_usage(expr1),
        },
        // Can't have await in the branches (parse error)
        Expr_::Eif(box (cond, _, _)) => check_await_usage(cond),
        Expr_::ExpressionTree(box nast::ExpressionTree {
            class: _,
            runtime_expr,
            free_vars: _,
        }) => check_await_usage(runtime_expr),
        // lvalues: shouldn't contain await or $$
        Expr_::List(_) => NoAwait,
    };
    match await_usage {
        Error(None) => Error(Some(pos.clone())),
        _ => await_usage,
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
    for_codegen: bool,
    // allow_con_of_seq allows concurrent compositions of sequential compositions, by over sequentialising.
    // This is experimental and for testing only.
    allow_con_of_seq: bool,
    leave_await: bool,
}

fn sequentialise(
    pos: Pos,
    con: Vec<(Lid, nast::Expr)>,
    seq: Vec<nast::Stmt>,
    mut tmps: Vec<Lid>,
) -> Vec<nast::Stmt> {
    if con.is_empty() && tmps.is_empty() {
        seq
    } else if con.is_empty() {
        let block = Stmt_::Block(Box::new((Some(tmps), Block(seq))));
        vec![Stmt(pos, block)]
    } else {
        tmps.extend(con.iter().map(|(lid, _)| lid.clone()));
        let awaitall = Stmt_::Awaitall(Box::new((con, Block(seq))));
        let block = Stmt_::Block(Box::new((
            Some(tmps),
            Block(vec![Stmt(pos.clone(), awaitall)]),
        )));
        vec![Stmt(pos, block)]
    }
}

fn sequentialise_stmt(
    pos: &Pos,
    elem: &mut nast::Stmt,
    stmt: Stmt_,
    con: Vec<(Lid, nast::Expr)>,
    mut seq: Vec<nast::Stmt>,
    tmps: Vec<Lid>,
) {
    seq.push(Stmt(pos.clone(), stmt));
    let mut stmts = sequentialise(pos.clone(), con, seq, tmps);
    if stmts.len() == 1 {
        *elem = stmts.remove(0)
    } else {
        *elem = Stmt(pos.clone(), Stmt_::Block(Box::new((None, Block(stmts)))))
    }
}

impl LiftAwait {
    fn gen_tmp_local(&mut self) -> LocalId {
        let name = format!(
            "{}lift_await{}",
            special_idents::TMP_VAR_PREFIX,
            self.tmp_var_counter
        );
        self.tmp_var_counter += 1;
        (0, name)
    }

    fn do_pipe(
        &mut self,
        e: &mut nast::Expr_,
        pos: &Pos,
        con: &mut Vec<(Lid, nast::Expr)>,
        seq: &mut Vec<nast::Stmt>,
        tmps: &mut Vec<Lid>,
    ) {
        match e {
            Expr_::Pipe(box (_, expr1, expr2)) => {
                let mut con1 = vec![];
                let mut seq1 = vec![];
                let mut tmps1 = vec![];
                self.extract_await(expr1, &mut con1, &mut seq1, &mut tmps1);
                if !ContainsAwait::default().check(expr2) {
                    con.append(&mut con1);
                    seq.append(&mut seq1);
                    tmps.append(&mut tmps1);
                } else {
                    let tmp1 = self.gen_tmp_local();
                    let mut replace = Some(Expr_::Lvar(Box::new(Lid(pos.clone(), tmp1.clone()))));
                    std::mem::swap(&mut replace, &mut self.replace_dd);
                    let mut con2 = vec![];
                    let mut seq2 = vec![];
                    let mut tmps2 = vec![];
                    self.extract_await(expr2, &mut con2, &mut seq2, &mut tmps2);
                    std::mem::swap(&mut replace, &mut self.replace_dd);
                    let mut lhs = Expr((), Pos::NONE, Expr_::Null);
                    let mut rhs = lhs.clone();
                    std::mem::swap(&mut lhs, expr1);
                    std::mem::swap(&mut rhs, expr2);
                    seq1.push(hack_stmt!(
                        pos = pos.clone(),
                        "#{lvar(clone(tmp1))} = #lhs;"
                    ));
                    let mut stmts1 = sequentialise(pos.clone(), con1, seq1, tmps1);
                    let tmp2 = self.gen_tmp_local();
                    // the rhs might have variables assigned in con2, and those get unset at the
                    // end of the awaitall, so we need to assign the rhs to a temporary that
                    // won't get unset.
                    seq2.push(hack_stmt!(
                        pos = pos.clone(),
                        "#{lvar(clone(tmp2))} = #rhs;"
                    ));
                    let mut stmts2 = sequentialise(pos.clone(), con2, seq2, tmps2);
                    let lid = Lid(pos.clone(), tmp2);
                    let mut lvar = Expr_::Lvar(Box::new(lid.clone()));
                    tmps.push(lid);
                    std::mem::swap(&mut lvar, e);
                    stmts1.append(&mut stmts2);
                    let block = Stmt(
                        pos.clone(),
                        Stmt_::Block(Box::new((
                            Some(vec![Lid(pos.clone(), tmp1)]),
                            Block(stmts1),
                        ))),
                    );
                    seq.append(&mut vec![block])
                }
            }
            _ => panic!(),
        }
    }

    // process the argument to an await expression
    fn do_await(
        &mut self,
        leave_await: bool,
        pos: &Pos,
        e: &mut nast::Expr_,
        con: &mut Vec<(Lid, nast::Expr)>,
        seq: &mut Vec<nast::Stmt>,
        tmps: &mut Vec<Lid>,
    ) {
        let mut con1 = vec![];
        let mut seq1 = vec![];
        let mut tmps1 = vec![];
        match e {
            Expr_::Await(box sub_expr) => {
                let mut awaited_expr = Expr((), Pos::NONE, Expr_::Null);
                std::mem::swap(&mut awaited_expr, sub_expr);
                self.extract_await(&mut awaited_expr, &mut con1, &mut seq1, &mut tmps1);
                if con1.is_empty() && seq1.is_empty() && tmps1.is_empty() {
                    if leave_await {
                        // If we're in a top-level await and there were no nested awaits, we
                        // don't need to do anything
                        std::mem::swap(&mut awaited_expr, sub_expr);
                        return;
                    }
                    // If there were no nested awaits, just lift the expression
                    let tmp = Lid(pos.clone(), self.gen_tmp_local());
                    let lvar = Expr_::Lvar(Box::new(tmp.clone()));
                    *e = lvar;
                    con.push((tmp, awaited_expr));
                    return;
                }
                let tmp1 = self.gen_tmp_local();
                seq1.push(hack_stmt!(
                    pos = pos.clone(),
                    "#{lvar(clone(tmp1))} = #awaited_expr;"
                ));
                let mut stmts1 = sequentialise(pos.clone(), con1, seq1, tmps1);
                let tmp2 = self.gen_tmp_local();
                let tmp2_lid = Lid(pos.clone(), tmp2.clone());
                let tmp3 = self.gen_tmp_local();
                let tmp3_lid = Lid(pos.clone(), tmp3.clone());
                let mut stmts2 = sequentialise(
                    pos.clone(),
                    vec![(tmp2_lid, hack_expr!("#{lvar(clone(tmp1))}"))],
                    vec![hack_stmt!("#{lvar(clone(tmp3))} = #{lvar(clone(tmp2))};")],
                    vec![],
                );
                stmts1.append(&mut stmts2);
                let block = Stmt(
                    pos.clone(),
                    Stmt_::Block(Box::new((
                        Some(vec![Lid(pos.clone(), tmp1)]),
                        Block(stmts1),
                    ))),
                );
                seq.append(&mut vec![block]);
                tmps.push(tmp3_lid.clone());
                *e = Expr_::Lvar(Box::new(tmp3_lid));
            }
            _ => panic!(),
        }
    }

    // extract_await(e, &mut con, &mut seq) transforms e by pulling awaits out into con' and
    // turning |> (with await on the rhs) into seq', such that it should be run as Awaitall (con') {seq'; e}.
    // It then appends con' and seq' to con and seq. It also appends any escaping temporaries to tmps.
    fn extract_await(
        &mut self,
        expr: &mut nast::Expr,
        con: &mut Vec<(Lid, nast::Expr)>,
        seq: &mut Vec<nast::Stmt>,
        tmps: &mut Vec<Lid>,
    ) {
        let leave_await = self.leave_await;
        self.leave_await = false;
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
            | Expr_::Nameof(_)
            | Expr_::FunctionPointer(_)
            | Expr_::ClassGet(_)
            | Expr_::Package(_) => {}

            // Expressions with exactly one sub-expression that we can lift an await out of
            Expr_::Invalid(box Some(expr))
            | Expr_::Clone(box expr)
            | Expr_::PrefixedString(box (_, expr))
            | Expr_::ReadonlyExpr(box expr)
            | Expr_::Cast(box (_, expr))
            | Expr_::Is(box (expr, _))
            | Expr_::As(box nast::As_ { expr, .. })
            | Expr_::Upcast(box (expr, _))
            | Expr_::Import(box (_, expr))
            | Expr_::Hole(box (expr, _, _, _))
            | Expr_::Yield(box Afield::AFvalue(expr))
            | Expr_::ETSplice(box nast::EtSplice {
                spliced_expr: expr, ..
            })
            | Expr_::Unop(box (_, expr))
            | Expr_::ArrayGet(box (expr, None)) => self.extract_await(expr, con, seq, tmps),

            // Expressions with exactly two sub-expressions that we can lift an await out of
            // and run concurrently
            Expr_::ArrayGet(box (expr1, Some(expr2)))
            | Expr_::Pair(box (_, expr1, expr2))
            | Expr_::ObjGet(box (expr1, expr2, _, _))
            | Expr_::Yield(box Afield::AFkvalue(expr1, expr2)) => {
                self.extract_await(expr1, con, seq, tmps);
                self.extract_await(expr2, con, seq, tmps)
            }
            Expr_::Binop(box nast::Binop { bop: _, lhs, rhs }) => {
                // If the operator is || or &&, there are syntactic restrictions that
                // there is no await on the rhs, and so it is safe to traverse it here
                // just to replace $$
                self.extract_await(lhs, con, seq, tmps);
                self.extract_await(rhs, con, seq, tmps)
            }
            Expr_::Assign(box (lhs, bop, rhs)) => {
                // If the operator is || or &&, there are syntactic restrictions that
                // there is no await on the rhs, and so it is safe to traverse it here
                // just to replace $$
                self.extract_await(lhs, con, seq, tmps);
                if bop.is_none() {
                    self.leave_await = leave_await;
                }
                self.extract_await(rhs, con, seq, tmps)
            }
            // Expressions with lists of sub-expressions that we can lift an await out of
            // and run concurrently
            Expr_::KeyValCollection(box (_, _, args)) => {
                for nast::Field(arg_k, arg_v) in args {
                    self.extract_await(arg_k, con, seq, tmps);
                    self.extract_await(arg_v, con, seq, tmps)
                }
            }
            Expr_::Tuple(args) | Expr_::String2(args) | Expr_::ValCollection(box (_, _, args)) => {
                for arg in args {
                    self.extract_await(arg, con, seq, tmps)
                }
            }
            Expr_::Shape(fields) => {
                for (_, expr) in fields {
                    self.extract_await(expr, con, seq, tmps)
                }
            }
            Expr_::Call(box nast::CallExpr {
                func,
                targs: _,
                args,
                unpacked_arg,
            }) => {
                self.extract_await(func, con, seq, tmps);
                for arg in args {
                    self.extract_await(arg.to_expr_mut(), con, seq, tmps)
                }
                if let Some(unpack) = unpacked_arg {
                    self.extract_await(unpack, con, seq, tmps)
                }
            }
            Expr_::New(box (nast::ClassId(_, _, cid), _, args, unpacked_arg, _)) => {
                match cid {
                    ClassId_::CIexpr(expr) => self.extract_await(expr, con, seq, tmps),
                    ClassId_::CIparent
                    | ClassId_::CIself
                    | ClassId_::CIstatic
                    | ClassId_::CI(_) => {}
                }
                for arg in args {
                    self.extract_await(arg, con, seq, tmps)
                }
                if let Some(unpack) = unpacked_arg {
                    self.extract_await(unpack, con, seq, tmps)
                }
            }
            Expr_::Xml(box (_, attrs, exprs)) => {
                for attr in attrs {
                    match attr {
                        XhpAttribute::XhpSpread(expr)
                        | XhpAttribute::XhpSimple(nast::XhpSimple { expr, .. }) => {
                            self.extract_await(expr, con, seq, tmps)
                        }
                    }
                }
                for expr in exprs {
                    self.extract_await(expr, con, seq, tmps);
                }
            }
            Expr_::Collection(box (_, _, afields)) => {
                for afield in afields {
                    match afield {
                        Afield::AFvalue(expr) => {
                            self.extract_await(expr, con, seq, tmps);
                        }
                        Afield::AFkvalue(expr1, expr2) => {
                            self.extract_await(expr1, con, seq, tmps);
                            self.extract_await(expr2, con, seq, tmps)
                        }
                    }
                }
            }
            // Await
            Expr_::Await(_) => self.do_await(leave_await, pos, e, con, seq, tmps),
            // Seq
            Expr_::Pipe(_) => self.do_pipe(e, pos, con, seq, tmps),
            Expr_::Eif(box (cond, t, f)) => {
                self.extract_await(cond, con, seq, tmps);
                if self.replace_dd.is_some() {
                    // Relying on syntactic checks that there are no await in t or f,
                    // but still need to traverse in case there is $$
                    if let Some(expr) = t {
                        self.extract_await(expr, con, seq, tmps)
                    }
                    self.extract_await(f, con, seq, tmps)
                }
            }

            Expr_::ExpressionTree(box nast::ExpressionTree {
                class: _,
                runtime_expr,
                free_vars: _,
            }) => self.extract_await(runtime_expr, con, seq, tmps),

            // lvalues: shouldn't contain await or $$
            Expr_::List(_) => {}
        }
    }

    fn check_and_extract_await(
        &mut self,
        env: &mut Env,
        expr: &mut nast::Expr,
        con: &mut Vec<(Lid, nast::Expr)>,
        seq: &mut Vec<nast::Stmt>,
        tmps: &mut Vec<Lid>,
    ) {
        use AwaitUsage::*;
        match check_await_usage(expr) {
            Error(p) if !self.allow_con_of_seq => {
                env.emit_error(NamingPhaseError::Parsing(ParsingError::ParsingError {
                    pos: p.unwrap_or(expr.1.clone()),
                    msg: "Nested or piped `await` expressions cannot be used alongside other concurrent awaits".to_string(),
                    quickfixes: vec![],
                }));
            }
            ContainsAwait | ConcurrentAwait | Sequential | Error(_) => {
                if self.for_codegen {
                    self.extract_await(expr, con, seq, tmps)
                }
            }
            NoAwait => {}
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
        let mut tmps = vec![];
        match &mut stmt_ {
            Stmt_::Expr(box expr) | Stmt_::Return(box Some(expr)) => {
                expr.accept(env, self.object())?;
                self.leave_await = true;
                self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps);
                self.leave_await = false;
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::Throw(box expr) => {
                expr.accept(env, self.object())?;
                self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps);
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::Return(box None) => {
                sequentialise_stmt(&pos, elem, stmt_, vec![], vec![], vec![]);
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
                    self.leave_await = true;
                    self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps);
                    self.leave_await = false;
                }
                block.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::If(box (expr, b1, b2)) => {
                expr.accept(env, self.object())?;
                self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps);
                b1.accept(env, self.object())?;
                b2.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::For(box (init_exprs, test, update, block)) => {
                for expr in init_exprs {
                    expr.accept(env, self.object())?;
                    self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps);
                }
                // No await in the test or update positions (parse error)
                test.accept(env, self.object())?;
                update.accept(env, self.object())?;
                block.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::Switch(box (case, block, default)) => {
                // No await in the case expression positions (parse error)
                case.accept(env, self.object())?;
                self.check_and_extract_await(env, case, &mut con, &mut seq, &mut tmps);
                block.accept(env, self.object())?;
                default.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::Foreach(box (expr, as_expr, block)) => {
                expr.accept(env, self.object())?;
                self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps);
                as_expr.accept(env, self.object())?;
                block.accept(env, self.object())?;
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            Stmt_::DeclareLocal(box (_, _, expr_opt)) => {
                if let Some(expr) = expr_opt {
                    expr.accept(env, self.object())?;
                    self.check_and_extract_await(env, expr, &mut con, &mut seq, &mut tmps)
                }
                sequentialise_stmt(&pos, elem, stmt_, con, seq, tmps);
                Ok(())
            }
            // await cannot appear in expression in do or while (parse error)
            Stmt_::Do(_)
            | Stmt_::While(_)
            | Stmt_::Block(_)
            | Stmt_::Try(_)
            | Stmt_::Match(_)
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
            Stmt_::Concurrent(Block(stmts)) => {
                for stmt in stmts.iter_mut() {
                    match stmt {
                        Stmt(_, Stmt_::Expr(box expr))
                        | Stmt(_, Stmt_::DeclareLocal(box (_, _, Some(expr)))) => {
                            expr.accept(env, self.object())?;
                            let is_await = expr.2.is_await();
                            match check_await_usage(expr) {
                                AwaitUsage::Sequential | AwaitUsage::Error(None) =>
                                    env.emit_error(NamingPhaseError::Parsing(ParsingError::ParsingError {
                                        pos: expr.1.clone(),
                                        msg:
                                            "Nested or piped `await` expressions cannot be used in concurrent block"
                                                .to_string(),
                                        quickfixes: vec![],
                                    })),
                                AwaitUsage::Error(Some(pos)) =>
                                    env.emit_error(NamingPhaseError::Parsing(ParsingError::ParsingError {
                                        pos: pos.clone(),
                                        msg:
                                            "Nested or piped `await` expressions cannot be used in concurrent block"
                                                .to_string(),
                                        quickfixes: vec![],
                                    })),
                                _ => {}
                            };
                            if self.for_codegen {
                                self.extract_await(expr, &mut con, &mut seq, &mut tmps);
                                if is_await {
                                    expr.2 = Expr_::Null;
                                }
                            }
                        }
                        Stmt(_, Stmt_::DeclareLocal(box (_, _, None))) => {}
                        _ => panic!(
                            "Concurrent block contains a statement that is not an expression-statement."
                        ),
                    }
                }
                if !self.for_codegen {
                    *elem = Stmt(pos, stmt_);
                    return Ok(());
                }
                let scope = con
                    .iter()
                    .map(|lifted_await| lifted_await.0.clone())
                    .collect();
                let mut tmp_vars = con.iter().map(|lifted_await| &lifted_await.0.1);
                let mut body_stmts = vec![];
                let mut assign_stmts = vec![];
                for n in stmts.drain(..) {
                    if !n.is_assign_expr() {
                        if n.is_null_expr() {
                            tmp_vars.next();
                        }
                        body_stmts.push(n.clone());
                        continue;
                    }
                    if let Stmt(
                        p1,
                        Stmt_::Expr(box Expr((), p2, Expr_::Assign(box (e1, op, e2)))),
                    ) = n
                    {
                        let tv = match tmp_vars.next() {
                            Some(tv) => tv,
                            None => continue,
                        };
                        let tmp_n = Expr::mk_lvar(&e2.1, &(tv.1));
                        if tmp_n.lvar_name() != e2.lvar_name() {
                            let new_n = Stmt::new(
                                p1.clone(),
                                Stmt_::Expr(Box::new(Expr::new(
                                    (),
                                    p2.clone(),
                                    Expr_::mk_assign(tmp_n.clone(), None, e2.clone()),
                                ))),
                            );
                            body_stmts.push(new_n);
                        }
                        let assign_stmt = Stmt::new(
                            p1.clone(),
                            Stmt_::Expr(Box::new(Expr::new(
                                (),
                                p2,
                                Expr_::mk_assign(e1, op, tmp_n),
                            ))),
                        );
                        assign_stmts.push(assign_stmt);
                    }
                }
                body_stmts.append(&mut assign_stmts);
                let stmt = Stmt::new(pos.clone(), Stmt_::mk_block(None, Block(body_stmts)));
                let awaitall_stmt =
                    Stmt::new(pos.clone(), Stmt_::mk_awaitall(con, Block(vec![stmt])));
                *elem = Stmt::new(
                    pos,
                    Stmt_::mk_block(Some(scope), Block(vec![awaitall_stmt])),
                );
                Ok(())
            }
        }
    }
}

pub fn elaborate_program(env: &mut Env, program: &mut nast::Program, for_codegen: bool) {
    let mut la = LiftAwait {
        for_codegen,
        ..Default::default()
    };
    la.visit_program(env, program).unwrap();
}

pub fn elaborate_fun_def(env: &mut Env, f: &mut nast::FunDef, for_codegen: bool) {
    let mut la = LiftAwait {
        for_codegen,
        ..Default::default()
    };
    la.visit_fun_def(env, f).unwrap();
}

pub fn elaborate_class_(env: &mut Env, c: &mut nast::Class_, for_codegen: bool) {
    let mut la = LiftAwait {
        for_codegen,
        ..Default::default()
    };
    la.visit_class_(env, c).unwrap();
}

pub fn elaborate_stmt(env: &mut Env, t: &mut nast::Stmt, for_codegen: bool) {
    let mut la = LiftAwait {
        for_codegen,
        ..Default::default()
    };
    la.visit_stmt(env, t).unwrap();
}

#[cfg(test)]
mod tests {
    use hack_macros::hack_expr;
    use hack_macros::hack_stmts;
    use nast::Def;
    use nast::Program;

    use super::*;

    pub fn elaborate_program_test(
        env: &mut Env,
        program: &mut nast::Program,
        allow_con_of_seq: bool,
    ) {
        let mut la = LiftAwait {
            allow_con_of_seq,
            for_codegen: true,
            ..Default::default()
        };
        la.visit_program(env, program).unwrap();
    }

    fn build_program(stmt: Stmt) -> Program {
        nast::Program(vec![Def::Stmt(Box::new(stmt))])
    }

    fn mk_lid(name: &str) -> Lid {
        Lid(Pos::NONE, (0, name.to_string()))
    }

    fn mk_lvar(lid: Lid) -> Expr {
        Expr((), Pos::NONE, Expr_::Lvar(Box::new(lid)))
    }

    fn mk_awaitall2(tmps: Vec<Lid>, con: Vec<(Lid, Expr)>, body: Block) -> Stmt {
        Stmt(
            Pos::NONE,
            Stmt_::Block(Box::new((
                Some(tmps),
                Block(vec![Stmt(
                    Pos::NONE,
                    Stmt_::Awaitall(Box::new((con, body))),
                )]),
            ))),
        )
    }

    fn mk_awaitall(con: Vec<(Lid, Expr)>, body: Block) -> Stmt {
        mk_awaitall2(con.iter().map(|(id, _)| id.clone()).collect(), con, body)
    }

    fn mk_block(lids: Vec<Lid>, body: Vec<Stmt>) -> Stmt {
        Stmt(Pos::NONE, Stmt_::Block(Box::new((Some(lids), Block(body)))))
    }

    #[test]
    fn no_await1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("$x0 * $x1 + $x2 * $x3;"));
        let res = build_program(hack_stmt!("$x0 * $x1 + $x2 * $x3;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn no_await2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("($x0 * $x1) |> ($x2 * $$);"));
        let res = build_program(hack_stmt!("($x0 * $x1) |> ($x2 * $$);"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn no_await3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("($x0 |> $$) + ($x2 |> $$);"));
        let res = build_program(hack_stmt!("($x0 |> $$) + ($x2 |> $$);"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_just_await1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x;"));
        let res = build_program(hack_stmt!("await $x;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_just_await2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("$y = await $x;"));
        let res = build_program(hack_stmt!("$y = await $x;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_con1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x + $y;"));
        let tmp = (0, "__tmp$lift_await0".to_string());
        let awaitall = mk_awaitall(
            vec![(Lid(Pos::NONE, tmp.clone()), hack_expr!("$x"))],
            Block(hack_stmts!("#{lvar(tmp)} + $y;")),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_con2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x0 * await $x1 + await $x2 * await $x3;"));
        let tmp0 = (0, "__tmp$lift_await0".to_string());
        let tmp1 = (0, "__tmp$lift_await1".to_string());
        let tmp2 = (0, "__tmp$lift_await2".to_string());
        let tmp3 = (0, "__tmp$lift_await3".to_string());
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
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_left1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x |> $$;"));
        let tmp = (0, "__tmp$lift_await0".to_string());
        let awaitall = mk_awaitall(
            vec![(Lid(Pos::NONE, tmp.clone()), hack_expr!("$x"))],
            Block(hack_stmts!("#{lvar(tmp)} |> $$;")),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_left2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("(await $x1 + await $x2) |> $$;"));
        let tmp0 = (0, "__tmp$lift_await0".to_string());
        let tmp1 = (0, "__tmp$lift_await1".to_string());
        let awaitall = mk_awaitall(
            vec![
                (Lid(Pos::NONE, tmp0.clone()), hack_expr!("$x1")),
                (Lid(Pos::NONE, tmp1.clone()), hack_expr!("$x2")),
            ],
            Block(hack_stmts!("#{lvar(tmp0)} + #{lvar(tmp1)} |> $$;")),
        );
        let res = build_program(awaitall);
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_right() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("$x |> await $$;"));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let awaitall = mk_awaitall(
            vec![(tmp1, tmp0_lvar.clone())],
            Block(hack_stmts!("#{clone(tmp2_lvar)} = #tmp1_lvar;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp2_lvar)));
        let b1 = mk_block(vec![tmp0], hack_stmts!("#tmp0_lvar = $x; #awaitall;"));
        let res = build_program(mk_block(vec![tmp2], hack_stmts!("#b1; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_both() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x |> await $$;"));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#{clone(tmp1_lvar)} = #tmp0_lvar;")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1_lvar)],
            Block(hack_stmts!("#{clone(tmp3_lvar)} = #tmp2_lvar;")),
        );
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp3_lvar)));
        let b1 = mk_block(vec![tmp1], hack_stmts!("#awaitall1; #awaitall2;"));
        let res = build_program(mk_block(vec![tmp3], hack_stmts!("#b1; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_both2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!(
            "(await $x + 1 |> await $$ + 2) |> await $$ + 3;"
        ));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lid("__tmp$lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let tmp5 = mk_lid("__tmp$lift_await5");
        let tmp5_lvar = mk_lvar(tmp5.clone());
        let tmp6 = mk_lid("__tmp$lift_await6");
        let tmp6_lvar = mk_lvar(tmp6.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#{clone(tmp1_lvar)} = #tmp0_lvar + 1;")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1_lvar)],
            Block(hack_stmts!("#{clone(tmp3_lvar)} = #tmp2_lvar + 2;")),
        );
        let b1 = mk_block(vec![tmp1], hack_stmts!("#awaitall1; #awaitall2;"));
        let b2 = mk_block(
            vec![tmp3],
            hack_stmts!("#b1; #{clone(tmp4_lvar)} = #tmp3_lvar;"),
        );
        let awaitall3 = mk_awaitall(
            vec![(tmp5, tmp4_lvar)],
            Block(hack_stmts!("#{clone(tmp6_lvar)} = #tmp5_lvar + 3;")),
        );
        let b3 = mk_block(vec![tmp4], hack_stmts!("#b2; #awaitall3;"));
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp6_lvar)));
        let res = build_program(mk_block(vec![tmp6], hack_stmts!("#b3; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_both3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!(
            "await $x + 1 |> (await $$ + 2 |> await $$ + 3);"
        ));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lid("__tmp$lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let tmp5 = mk_lid("__tmp$lift_await5");
        let tmp5_lvar = mk_lvar(tmp5.clone());
        let tmp6 = mk_lid("__tmp$lift_await6");
        let tmp6_lvar = mk_lvar(tmp6.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!(
                "#{clone(tmp1_lvar)} = #{clone(tmp0_lvar)} + 1;"
            )),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1_lvar)],
            Block(hack_stmts!(
                "#{clone(tmp3_lvar)} = #{clone(tmp2_lvar)} + 2;"
            )),
        );
        let awaitall3 = mk_awaitall(
            vec![(tmp4, tmp3_lvar)],
            Block(hack_stmts!(
                "#{clone(tmp5_lvar)} = #{clone(tmp4_lvar)} + 3;"
            )),
        );
        let b1 = mk_block(vec![tmp3], hack_stmts!("#awaitall2; #awaitall3;"));
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp6_lvar.clone())));
        let b2 = mk_block(vec![tmp5], hack_stmts!("#b1; #tmp6_lvar = #tmp5_lvar;"));
        let b3 = mk_block(vec![tmp1], hack_stmts!("#awaitall1; #b2;"));
        let res = build_program(mk_block(vec![tmp6], hack_stmts!("#b3; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq_of_con() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!(
            "(await $x + await $y) |> (await $$ + await $$);"
        ));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lid("__tmp$lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let tmp5 = mk_lid("__tmp$lift_await5");
        let tmp5_lvar = mk_lvar(tmp5.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x")), (tmp1, hack_expr!("$y"))],
            Block(hack_stmts!(
                "#{clone(tmp2_lvar)} = #{clone(tmp0_lvar)} + #{clone(tmp1_lvar)};"
            )),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp3, tmp2_lvar.clone()), (tmp4, tmp2_lvar)],
            Block(hack_stmts!(
                "#{clone(tmp5_lvar)} = #tmp3_lvar + #tmp4_lvar;"
            )),
        );
        let b1 = mk_block(vec![tmp2], hack_stmts!("#awaitall1; #awaitall2;"));
        let stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp5_lvar)));
        let res = build_program(mk_block(vec![tmp5], hack_stmts!("#b1; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_con_of_seq() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("await $x + (await $y |> await $$);"));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lid("__tmp$lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp1, hack_expr!("$y"))],
            Block(hack_stmts!("#{clone(tmp2_lvar)} = #{clone(tmp1_lvar)};")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp3, tmp2_lvar)],
            Block(hack_stmts!("#{clone(tmp4_lvar)} = #{clone(tmp3_lvar)};")),
        );
        let _stmt = Stmt(Pos::NONE, Stmt_::Expr(Box::new(tmp4_lvar.clone())));
        let b1 = mk_block(vec![tmp2], hack_stmts!("#awaitall1; #awaitall2;"));
        let awaitall3 = mk_awaitall2(
            vec![tmp4, tmp0.clone()],
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#b1; #tmp0_lvar + #tmp4_lvar;")),
        );
        let res = build_program(hack_stmt!("#awaitall3;"));
        self::elaborate_program_test(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_nested_await1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("g(await f(await $x));"));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x"))],
            Block(hack_stmts!("#{clone(tmp1_lvar)} = f(#tmp0_lvar);")),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp2, tmp1_lvar)],
            Block(hack_stmts!("#{clone(tmp3_lvar)} = #tmp2_lvar;")),
        );
        let stmt = hack_stmt!("g(#tmp3_lvar);");
        let b1 = mk_block(vec![tmp1], hack_stmts!("#awaitall1; #awaitall2;"));
        // Same result as "await $x |> await $$"
        let res = build_program(mk_block(vec![tmp3], hack_stmts!("#b1; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_nested_await2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmt!("h (await g(await f(await $x, await $y)));"));
        let tmp0 = mk_lid("__tmp$lift_await0");
        let tmp0_lvar = mk_lvar(tmp0.clone());
        let tmp1 = mk_lid("__tmp$lift_await1");
        let tmp1_lvar = mk_lvar(tmp1.clone());
        let tmp2 = mk_lid("__tmp$lift_await2");
        let tmp2_lvar = mk_lvar(tmp2.clone());
        let tmp3 = mk_lid("__tmp$lift_await3");
        let tmp3_lvar = mk_lvar(tmp3.clone());
        let tmp4 = mk_lid("__tmp$lift_await4");
        let tmp4_lvar = mk_lvar(tmp4.clone());
        let tmp5 = mk_lid("__tmp$lift_await5");
        let tmp5_lvar = mk_lvar(tmp5.clone());
        let tmp6 = mk_lid("__tmp$lift_await6");
        let tmp6_lvar = mk_lvar(tmp6.clone());
        let tmp7 = mk_lid("__tmp$lift_await7");
        let tmp7_lvar = mk_lvar(tmp7.clone());
        let awaitall1 = mk_awaitall(
            vec![(tmp0, hack_expr!("$x")), (tmp1, hack_expr!("$y"))],
            Block(hack_stmts!(
                "#{clone(tmp2_lvar)} = f(#tmp0_lvar, #tmp1_lvar);"
            )),
        );
        let awaitall2 = mk_awaitall(
            vec![(tmp3, tmp2_lvar)],
            Block(hack_stmts!("#{clone(tmp4_lvar)} = #tmp3_lvar;")),
        );
        let b1 = mk_block(vec![tmp2], hack_stmts!("#awaitall1; #awaitall2;"));
        let b2 = mk_block(
            vec![tmp4],
            hack_stmts!("#b1; #{clone(tmp5_lvar)} = g(#tmp4_lvar);"),
        );
        let awaitall3 = mk_awaitall(
            vec![(tmp6, tmp5_lvar)],
            Block(hack_stmts!("#{clone(tmp7_lvar)} = #tmp6_lvar;")),
        );
        let b3 = mk_block(vec![tmp5], hack_stmts!("#b2; #awaitall3;"));
        let stmt = hack_stmt!("h(#tmp7_lvar);");
        let res = build_program(mk_block(vec![tmp7], hack_stmts!("#b3; #stmt;")));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }
}
