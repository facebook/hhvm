// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hack::Builtin;
use hack::Hhbc;
use ir::ArrayKey;
use ir::ClassId;
use ir::StringInterner;
use ir::TypedValue;
use itertools::Itertools;
use textual::Const;
use textual::Expr;

use crate::hack;
use crate::mangle::TypeName;
use crate::textual;
use crate::util;

pub(crate) fn typed_value_expr(tv: &TypedValue, strings: &StringInterner) -> Expr {
    match *tv {
        TypedValue::Uninit => textual::Expr::null(),
        TypedValue::Int(n) => hack::expr_builtin(Builtin::Int, [Expr::Const(Const::Int(n))]),
        TypedValue::Bool(false) => hack::expr_builtin(Builtin::Bool, [Expr::Const(Const::False)]),
        TypedValue::Bool(true) => hack::expr_builtin(Builtin::Bool, [Expr::Const(Const::True)]),
        TypedValue::Float(f) => hack::expr_builtin(Builtin::Float, [Expr::Const(Const::Float(f))]),
        TypedValue::LazyClass(cid) => Expr::Const(Const::LazyClass(TypeName::Class(cid))),
        TypedValue::String(s) => {
            let s = util::escaped_string(&strings.lookup_bytes(s));
            hack::expr_builtin(Builtin::String, [Expr::Const(Const::String(s))])
        }
        TypedValue::Null => textual::Expr::null(),
        TypedValue::Vec(ref v) => {
            let args: Vec<Expr> = v
                .iter()
                .map(|value| typed_value_expr(value, strings))
                .collect_vec();
            hack::expr_builtin(Builtin::Hhbc(Hhbc::NewVec), args)
        }
        TypedValue::Keyset(ir::KeysetValue(ref k)) => {
            let args: Vec<Expr> = k
                .iter()
                .map(|value| array_key_expr(value, strings))
                .collect_vec();
            hack::expr_builtin(Builtin::Hhbc(Hhbc::NewKeysetArray), args)
        }
        TypedValue::Dict(ir::DictValue(ref d)) => {
            let args = d
                .iter()
                .flat_map(|(k, v)| {
                    let k = array_key_expr(k, strings);
                    let v = typed_value_expr(v, strings);
                    [k, v]
                })
                .collect_vec();
            hack::expr_builtin(Builtin::NewDict, args)
        }
    }
}

pub(crate) fn array_key_expr(ak: &ArrayKey, strings: &StringInterner) -> Expr {
    match *ak {
        ArrayKey::Int(n) => hack::expr_builtin(Builtin::Int, [Expr::Const(Const::Int(n))]),
        ArrayKey::String(s) | ArrayKey::LazyClass(ClassId { id: s }) => {
            let s = util::escaped_string(&strings.lookup_bytes(s));
            hack::expr_builtin(Builtin::String, [Expr::Const(Const::String(s))])
        }
    }
}
