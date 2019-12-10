// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_class_expr_rust as ast_class_expr;
use ast_scope_rust as ast_scope;
use env::emitter::Emitter;
use hhbc_id_rust::Id;
use hhbc_string_utils_rust as string_utils;
use naming_special_names_rust::{math, members, typehints};
use oxidized::{
    aast_visitor::{visit_mut, NodeMut, VisitorMut},
    ast as tast, ast_defs,
};
use runtime::TypedValue;

enum Radix {
    Oct,
    Hex,
    Dec,
    Bin,
}

fn radix(s: &str) -> Radix {
    let s = s.as_bytes();
    if s.len() > 1 && (s[0] as char) == '0' {
        match s[1] as char {
            'b' | 'B' => Radix::Bin,
            'x' | 'X' => Radix::Hex,
            _ => Radix::Oct,
        }
    } else {
        Radix::Dec
    }
}

fn try_type_intlike(s: &str) -> Option<i64> {
    match radix(s) {
        Radix::Dec => s.parse().ok(),
        Radix::Bin => i64::from_str_radix(&s[2..], 2).ok(),
        Radix::Oct => i64::from_str_radix(&s[1..], 8).ok(),
        Radix::Hex => i64::from_str_radix(&s[2..], 16).ok(),
    }
}

fn class_const_to_typed_value(
    emitter: &Emitter,
    cid: &tast::ClassId,
    id: &tast::Pstring,
) -> Option<TypedValue> {
    if id.1 == members::M_CLASS {
        let cexpr = ast_class_expr::ClassExpr::class_id_to_class_expr(
            emitter,
            false,
            true,
            &ast_scope::Scope::toplevel(),
            cid.clone(),
        );
        if let ast_class_expr::ClassExpr::Id(ast_defs::Id(_, cname)) = cexpr {
            let cname = hhbc_id_rust::class::Type::from_ast_name(&cname).into();
            return Some(TypedValue::String(cname));
        }
    }
    None
}

fn array_to_typed_value(fields: &Vec<tast::Afield>) -> Option<TypedValue> {
    Some(TypedValue::Array(if fields.is_empty() {
        vec![]
    } else {
        // TODO(hrust): for the purposes of constant folding (fold_program entry point),
        // it only matters whether an array is empty or not, so putting a
        // dummy values here. It will need to be implemented for other callers
        // of this module (in emit_expression)
        vec![(TypedValue::Uninit, TypedValue::Uninit)]
    }))
}

pub fn expr_to_opt_typed_value(emitter: &Emitter, expr: &tast::Expr) -> Option<TypedValue> {
    use TypedValue::*;
    match &expr.1 {
        tast::Expr_::Int(s) => Some(Int(try_type_intlike(&s).unwrap_or(std::i64::MAX))),
        tast::Expr_::True => Some(Bool(true)),
        tast::Expr_::False => Some(Bool(false)),
        tast::Expr_::Null => Some(Null),
        tast::Expr_::String(s) => Some(String(s.to_owned())),
        tast::Expr_::Float(s) => s.parse().ok().map(|x| Float(x)),
        tast::Expr_::Id(id) if id.1 == math::NAN => Some(Float(std::f64::NAN)),
        tast::Expr_::Id(id) if id.1 == math::INF => Some(Float(std::f64::INFINITY)),
        tast::Expr_::ClassConst(x) => class_const_to_typed_value(emitter, &x.0, &x.1),
        tast::Expr_::Array(fields) => array_to_typed_value(&fields),
        _ => None,
    }
}

fn cast_value(hint: &tast::Hint_, v: TypedValue) -> Option<TypedValue> {
    match hint {
        tast::Hint_::Happly(ast_defs::Id(_, id), args) if args.is_empty() => {
            let id = string_utils::strip_hh_ns(id);
            if id == typehints::BOOL {
                v.cast_to_bool()
            } else if id == typehints::STRING {
                v.cast_to_string()
            } else if id == typehints::FLOAT {
                v.cast_to_float()
            } else {
                None
            }
        }
        _ => None,
    }
}

fn unop_on_value(unop: &ast_defs::Uop, v: TypedValue) -> Option<TypedValue> {
    match unop {
        ast_defs::Uop::Unot => v.not(),
        ast_defs::Uop::Uplus => v.add(&TypedValue::Int(0)),
        ast_defs::Uop::Uminus => v.neg(),
        ast_defs::Uop::Utild => v.bitwise_not(),
        ast_defs::Uop::Usilence => Some(v.clone()),
        _ => None,
    }
}

fn binop_on_values(binop: &ast_defs::Bop, v1: TypedValue, v2: TypedValue) -> Option<TypedValue> {
    use ast_defs::Bop::*;
    match binop {
        Dot => v1.concat(v2),
        Plus => v1.add(&v2),
        Minus => v1.sub(&v2),
        Star => v1.mul(&v2),
        Ltlt => v1.shift_left(&v2),
        Slash => v1.div(&v2),
        Bar => v1.bitwise_or(&v2),
        _ => None,
    }
}

fn value_to_expr(v: TypedValue) -> tast::Expr_ {
    use tast::*;
    use TypedValue::*;
    match v {
        Int(i) => Expr_::Int(i.to_string()),
        Float(i) => Expr_::Float(hhbc_string_utils_rust::float::to_string(i)),
        Bool(false) => Expr_::False,
        Bool(true) => Expr_::True,
        String(s) => Expr_::String(s),
        _ => panic!("TODO"),
    }
}

struct FolderVisitor {}

impl VisitorMut for FolderVisitor {
    type Context = Emitter;
    type Ex = ast_defs::Pos;
    type Fb = ();
    type En = ();
    type Hi = ();

    fn object(
        &mut self,
    ) -> &mut dyn VisitorMut<
        Context = Self::Context,
        Ex = Self::Ex,
        Fb = Self::Fb,
        En = Self::En,
        Hi = Self::Hi,
    > {
        self
    }

    fn visit_expr_(&mut self, c: &mut Self::Context, p: &mut tast::Expr_) {
        p.recurse(c, self.object());
        let new_p = match p {
            tast::Expr_::Cast(e) => expr_to_opt_typed_value(c, &e.1)
                .and_then(|v| cast_value(&(e.0).1, v))
                .map(&value_to_expr),
            tast::Expr_::Unop(e) => expr_to_opt_typed_value(c, &e.1)
                .and_then(|v| unop_on_value(&e.0, v))
                .map(&value_to_expr),
            tast::Expr_::Binop(e) => expr_to_opt_typed_value(c, &e.1).and_then(|v1| {
                expr_to_opt_typed_value(c, &e.2)
                    .and_then(|v2| binop_on_values(&e.0, v1, v2).map(&value_to_expr))
            }),
            _ => None,
        };
        if let Some(new_p) = new_p {
            *p = new_p
        }
    }
}

pub fn fold_program(p: &mut tast::Program, e: &mut Emitter) {
    visit_mut(&mut FolderVisitor {}, e, p);
}
