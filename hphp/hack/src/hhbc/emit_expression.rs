// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope_rust::Scope;
use emit_type_constant_rust as emit_type_constant;
use env::Env;
use hhbc_ast_rust::*;
use instruction_sequence_rust::InstrSeq;
use local_rust as local;
use naming_special_names_rust::special_idents;
use options::Options;
use oxidized::{aast, local_id};

use std::collections::BTreeMap;

pub struct EmitJmpResult {
    // generated instruction sequence
    instrs: InstrSeq,
    // does instruction sequence fall through
    is_fallthrough: bool,
    // was label associated with emit operation used
    is_label_used: bool,
}

pub enum LValOp {
    Set,
    SetOp(EqOp),
    IncDec(IncdecOp),
    Unset,
}

pub fn is_local_this(env: &Env, lid: &local_id::LocalId) -> bool {
    local_id::get_name(lid) == special_idents::THIS
        && Scope::has_this(&env.scope)
        && !Scope::is_toplevel(&env.scope)
}

mod inout_locals {
    use crate::*;
    use oxidized::{aast_defs::Lid, aast_visitor, aast_visitor::Node, ast as tast, ast_defs};
    use std::{
        collections::{hash_map::Entry, HashMap},
        marker::PhantomData,
    };

    struct AliasInfo {
        first_inout: usize,
        last_write: usize,
        num_uses: usize,
    }

    impl Default for AliasInfo {
        fn default() -> Self {
            AliasInfo {
                first_inout: std::usize::MAX,
                last_write: std::usize::MIN,
                num_uses: 0,
            }
        }
    }

    impl AliasInfo {
        pub fn add_inout(&mut self, i: usize) {
            if i < self.first_inout {
                self.first_inout = i;
            }
        }

        pub fn add_write(&mut self, i: usize) {
            if i > self.last_write {
                self.last_write = i;
            }
        }

        pub fn add_use(&mut self) {
            self.num_uses += 1
        }

        pub fn in_range(&self, i: usize) -> bool {
            i > self.first_inout || i <= self.last_write
        }

        pub fn has_single_ref(&self) -> bool {
            self.num_uses < 2
        }
    }

    type AliasInfoMap = HashMap<String, AliasInfo>;

    fn add_write(name: String, i: usize, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_write(i);
    }

    fn add_inout(name: String, i: usize, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_inout(i);
    }

    fn add_use(name: String, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_use();
    }

    // determines if value of a local 'name' that appear in parameter 'i'
    // should be saved to local because it might be overwritten later
    fn should_save_local_value(name: String, i: usize, aliases: &AliasInfoMap) -> bool {
        aliases.get(&name).map_or(false, |alias| alias.in_range(i))
    }

    fn should_move_local_value(local: &local::Type, aliases: &AliasInfoMap) -> bool {
        match local {
            local::Type::Named(name) => aliases
                .get(&**name)
                .map_or(true, |alias| alias.has_single_ref()),
            local::Type::Unnamed(_) => false,
        }
    }

    fn collect_written_variables(env: &Env, args: &Vec<tast::Expr>) -> AliasInfoMap {
        let mut acc = HashMap::new();
        args.iter()
            .enumerate()
            .for_each(|(i, arg)| handle_arg(env, true, i, arg, &mut acc));
        acc
    }

    fn handle_arg(env: &Env, is_top: bool, i: usize, arg: &tast::Expr, acc: &mut AliasInfoMap) {
        use tast::{Expr, Expr_};

        let Expr(_, e) = arg;
        match e {
            Expr_::Callconv(x) => {
                if let (ast_defs::ParamKind::Pinout, Expr(_, Expr_::Lvar(lid))) = &**x {
                    let Lid(_, lid) = &**lid;
                    if !is_local_this(env, &lid) {
                        add_use(lid.1.to_string(), acc);
                        if is_top {
                            add_inout(lid.1.to_string(), i, acc);
                        } else {
                            add_write(lid.1.to_string(), i, acc);
                        }
                    }
                }
            }
            Expr_::Lvar(lid) => {
                let Lid(_, (_, id)) = &**lid;
                add_use(id.to_string(), acc);
            }
            _ => {
                // dive into argument value
                aast_visitor::visit(
                    &mut Visitor {
                        phantom: PhantomData,
                    },
                    &mut Ctx { state: acc, env, i },
                    arg,
                );
            }
        }
    }

    struct Visitor<'a> {
        phantom: PhantomData<&'a str>,
    }

    pub struct Ctx<'a> {
        state: &'a mut AliasInfoMap,
        env: &'a Env,
        i: usize,
    }

    impl<'a> aast_visitor::Visitor for Visitor<'a> {
        type Context = Ctx<'a>;
        type Ex = ast_defs::Pos;
        type Fb = ();
        type En = ();
        type Hi = ();

        fn object(
            &mut self,
        ) -> &mut dyn aast_visitor::Visitor<
            Context = Self::Context,
            Ex = Self::Ex,
            Fb = Self::Fb,
            En = Self::En,
            Hi = Self::Hi,
        > {
            self
        }

        fn visit_expr_(&mut self, c: &mut Self::Context, p: &tast::Expr_) {
            p.recurse(c, self.object());
            match p {
                tast::Expr_::Binop(expr) => {
                    let (bop, left, _) = &**expr;
                    if let ast_defs::Bop::Eq(_) = bop {
                        collect_lvars_hs(c, left)
                    }
                }
                tast::Expr_::Unop(expr) => {
                    let (uop, e) = &**expr;
                    match uop {
                        ast_defs::Uop::Uincr | ast_defs::Uop::Udecr => collect_lvars_hs(c, e),
                        _ => (),
                    }
                }
                tast::Expr_::Lvar(expr) => {
                    let Lid(_, (_, id)) = &**expr;
                    add_use(id.to_string(), &mut c.state);
                }
                tast::Expr_::Call(expr) => {
                    let (_, _, _, args, uarg) = &**expr;
                    args.iter()
                        .for_each(|arg| handle_arg(&c.env, false, c.i, arg, &mut c.state));
                    uarg.as_ref()
                        .map(|arg| handle_arg(&c.env, false, c.i, arg, &mut c.state));
                }
                _ => (),
            }
        }
    }

    // collect lvars on the left hand side of '=' operator
    fn collect_lvars_hs(ctx: &mut Ctx, expr: &tast::Expr) {
        let tast::Expr(_, e) = expr;
        match &*e {
            tast::Expr_::Lvar(lid) => {
                let Lid(_, lid) = &**lid;
                if !is_local_this(&ctx.env, &lid) {
                    add_use(lid.1.to_string(), &mut ctx.state);
                    add_write(lid.1.to_string(), ctx.i, &mut ctx.state);
                }
            }
            tast::Expr_::List(exprs) => exprs.iter().for_each(|expr| collect_lvars_hs(ctx, expr)),
            _ => (),
        }
    }
}

pub fn get_type_structure_for_hint(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: aast::Hint,
) -> InstrSeq {
    let _tv = emit_type_constant::hint_to_type_constant(opts, tparams, targ_map, hint);
    unimplemented!("TODO(hrust) after porting most of emit_adata")
}
