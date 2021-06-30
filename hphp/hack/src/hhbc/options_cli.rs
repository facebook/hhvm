// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! A Command Line Interface (CLI) adaptor to JSON serde.
//!
//! For options that don't parse as strings (excluding flags), this module
//! defines to-JSON parse functions returned by [to_json(json_key)].

use serde_json::{json, value::Value as Json};

use std::collections::HashMap;

macro_rules! impl_CANON_BY_ALIAS {
    ($($name:expr => $field:expr),* ,) => {{
        let mut m = HashMap::new();
        $( m.insert($name, $field); )*
        m
    }}
}

lazy_static! {
    /// Each config (JSON) key is implicitly: prefix + lower-cased flag name
    /// the exceptions are overridden here for config list (-v KEY=VAL).
    pub static ref CANON_BY_ALIAS: HashMap<&'static str, &'static str> =
        impl_CANON_BY_ALIAS!(
            // group 1: obtained by removing underscores from JSON field names
            "hack.compiler.constantfolding" => "hack.compiler.constant_folding",
            "hack.compiler.optimizenullcheck" => "hack.compiler.optimize_null_checks",
            // group 2: also different prefix without any obvious rule
            "eval.emit_meth_caller_func_pointers" => "hhvm.emit_meth_caller_func_pointers",
            "eval.jitenablerenamefunction" => "hhvm.jit_enable_rename_function",
            "eval.logexterncompilerperf" => "hhvm.log_extern_compiler_perf",
            "eval.enableintrinsicsextension" => "hhvm.enable_intrinsics_extension",
            // group 3: we could ignore hhvm. part of prefix in deser.
            "hack.lang.disable_lval_as_an_expression" => "hhvm.hack.lang.disable_lval_as_an_expression",
            // group 4: combination of group 3 & 1
            "hack.lang.phpism.disablenontopleveldeclarations" => "hhvm.hack.lang.phpism.disable_nontoplevel_declarations",
            // group 5: we could assume "hack." between "hhvm." and "lang."
            "hhvm.lang.enable_class_level_where_clauses" => "hhvm.hack.lang.enable_class_level_where_clauses",
            "hhvm.lang.disable_legacy_soft_typehints" => "hhvm.hack.lang.disable_legacy_soft_typehints",
            "hhvm.lang.allow_new_attribute_syntax" => "hhvm.hack.lang.allow_new_attribute_syntax",
            "hhvm.lang.disable_legacy_attribute_syntax" => "hhvm.hack.lang.disable_legacy_attribute_syntax",
            "hhvm.lang.disallow_func_ptrs_in_constants" => "hhvm.hack.lang.disallow_func_ptrs_in_constants",
            // group 6: combination of group 5 & 1
            "hhvm.lang.constdefaultfuncargs" => "hhvm.hack.lang.const_default_func_args",
            "hhvm.lang.constdefaultlambdaargs" => "hhvm.hack.lang.const_default_lambda_args",
            "hhvm.lang.abstractstaticprops" => "hhvm.hack.lang.abstract_static_props",
            "hhvm.lang.disableunsetclassconst" => "hhvm.hack.lang.disable_unset_class_const",
        );
        // TODO(leoo) use const-concat & unsnakecase macro via proc_macro_hack
}

pub type ParseFn = fn(&str) -> Option<Json>;

/// Given a config key as in JSON (not CLI) returns a to-JSON parse function. E.g.,
/// for "hhvm.hack_arr_compat_notices" (not "eval.hackarrcompatnotices"), it
/// returns a boolean parser.
pub fn to_json(key: &str) -> ParseFn {
    *BY_JSON_KEY.get(key).unwrap_or(&(parse_str as ParseFn))
}

lazy_static! {
    static ref BY_JSON_KEY: HashMap<&'static str, ParseFn> = {
        let mut m = HashMap::new();
        m.insert("hhvm.include_roots", parse_csv_keyval_strs as ParseFn);
        m
    };
}

fn parse_str(s: &str) -> Option<Json> {
    Some(json!(s))
}

fn parse_csv_keyval_strs(s: &str) -> Option<Json> {
    let mut ret = json!({});
    ret.as_object_mut().map(|m| {
        for keyval in s.split(",") {
            if let Some(sep_idx) = keyval.find(':') {
                let (key, val) = keyval.split_at(sep_idx);
                let val = &val[1..];
                m.insert(key.to_owned(), json!(val));
            }
        }
    });
    Some(ret)
}
