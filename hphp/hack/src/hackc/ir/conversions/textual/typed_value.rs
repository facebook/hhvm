// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hack::Builtin;
use hack::Hhbc;
use ir::TypedValue;
use itertools::Itertools;
use textual::Const;
use textual::Expr;

use crate::hack;
use crate::mangle::TypeName;
use crate::textual;
use crate::util;

pub(crate) fn typed_value_expr(tv: &TypedValue) -> Expr {
    match *tv {
        TypedValue::Uninit => textual::Expr::null(),
        TypedValue::Int(n) => hack::expr_builtin(Builtin::Int, [Expr::Const(Const::Int(n))]),
        TypedValue::Bool(false) => hack::expr_builtin(Builtin::Bool, [Expr::Const(Const::False)]),
        TypedValue::Bool(true) => hack::expr_builtin(Builtin::Bool, [Expr::Const(Const::True)]),
        TypedValue::Float(f) => hack::expr_builtin(Builtin::Float, [Expr::Const(Const::Float(f))]),
        TypedValue::LazyClass(cid) => Expr::Const(Const::LazyClass(TypeName::Class(cid))),
        TypedValue::String(s) => {
            let s = util::escaped_string(s.as_bytes());
            hack::expr_builtin(Builtin::String, [Expr::Const(Const::String(s))])
        }
        TypedValue::Null => textual::Expr::null(),
        TypedValue::Vec(ref v) => {
            let args: Vec<Expr> = v.iter().map(typed_value_expr).collect_vec();
            hack::expr_builtin(Builtin::Hhbc(Hhbc::NewVec), args)
        }
        TypedValue::Keyset(ref k) => {
            let args: Vec<Expr> = k.iter().map(typed_value_expr).collect_vec();
            hack::expr_builtin(Builtin::Hhbc(Hhbc::NewKeysetArray), args)
        }
        TypedValue::Dict(ref d) => {
            let args = d
                .iter()
                .flat_map(|e| {
                    let k = typed_value_expr(&e.key);
                    let v = typed_value_expr(&e.value);
                    [k, v]
                })
                .collect_vec();
            hack::expr_builtin(Builtin::NewDict, args)
        }
    }
}
