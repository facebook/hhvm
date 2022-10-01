// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::Path;

use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::global_options::GlobalOptions;
use sha1::Digest;
use sha1::Sha1;

pub const FILE_PATH_RELATIVE_TO_ROOT: &str = ".hhconfig";

/// For now, this struct only contains the parts of .hhconfig which
/// have been needed in Rust tools.
#[derive(Debug, Clone, Default)]
pub struct HhConfig {
    pub version: Option<String>,

    /// List of regex patterns of root-relative paths to ignore.
    pub ignored_paths: Vec<String>,

    /// SHA1 Hash of the .hhconfig file contents.
    pub hash: String,

    pub opts: GlobalOptions,

    /// Config file lines that did not match any setting known to this parser.
    pub unknown: Vec<String>,
}

impl HhConfig {
    pub fn from_root(root: &Path) -> std::io::Result<Self> {
        let hhconfig_path = root.join(FILE_PATH_RELATIVE_TO_ROOT);
        let hhconfig_contents = std::fs::read(&hhconfig_path)?;
        Ok(Self::from_slice(&hhconfig_contents))
    }

    pub fn from_slice(bytes: &[u8]) -> Self {
        use bstr::ByteSlice;

        let mut c = Self::default();
        for line in bytes.lines() {
            if matches!(line.get(0), Some(b'#')) {
                continue;
            }
            let mut key_value_iter = line.splitn(2, |&c| c == b'=');
            let key = match key_value_iter.next() {
                Some(key) => key.trim(),
                None => continue,
            };
            let value = match key_value_iter.next() {
                Some(value) => value.trim(),
                None => continue,
            };

            let go = &mut c.opts;
            match key {
                b"auto_namespace_map" => {
                    let map: BTreeMap<String, String> = parse_json(value);
                    go.po_auto_namespace_map = map.into_iter().collect();
                }
                b"disable_xhp_element_mangling" => {
                    go.po_disable_xhp_element_mangling = parse_json(value);
                }
                b"disable_xhp_children_declarations" => {
                    go.po_disable_xhp_children_declarations = parse_json(value);
                }
                b"interpret_soft_types_as_like_types" => {
                    go.po_interpret_soft_types_as_like_types = parse_json(value);
                }
                b"everything_sdt" => {
                    go.tco_everything_sdt = parse_json(value);
                }
                b"deregister_php_stdlib" => {
                    go.po_deregister_php_stdlib = parse_json(value);
                }
                b"version" => {
                    c.version = Some(parse_string(value));
                }
                b"ignored_paths" => {
                    c.ignored_paths = parse_json(value);
                }
                b"enable_experimental_tc_features" => {
                    go.tco_experimental_features = parse_sset(value);
                }
                b"enable_xhp_class_modifier" => {
                    go.po_enable_xhp_class_modifier = parse_json(value);
                }
                b"disallow_invalid_arraykey" => {
                    go.tco_disallow_invalid_arraykey = parse_json(value);
                }
                b"check_xhp_attribute" => {
                    go.tco_check_xhp_attribute = parse_json(value);
                }
                b"disallow_silence" => {
                    go.po_disallow_silence = parse_json(value);
                }
                b"check_redundant_generics" => {
                    go.tco_check_redundant_generics = parse_json(value);
                }
                b"disallow_func_ptrs_in_constants" => {
                    go.po_disallow_func_ptrs_in_constants = parse_json(value);
                }
                b"disallow_fun_and_cls_meth_pseudo_funcs" => {
                    go.po_disallow_fun_and_cls_meth_pseudo_funcs = parse_json(value);
                }
                b"enable_strict_string_concat_interp" => {
                    go.tco_enable_strict_string_concat_interp = parse_json(value);
                }
                b"disallow_inst_meth" => {
                    go.po_disallow_inst_meth = parse_json(value);
                }
                b"allowed_expression_tree_visitors" => {
                    go.tco_allowed_expression_tree_visitors = parse_svec(value);
                }
                b"math_new_code" => {
                    go.tco_math_new_code = parse_json(value);
                }
                b"explicit_consistent_constructors" => {
                    go.tco_explicit_consistent_constructors = parse_json(value);
                }
                b"enable_strict_const_semantics" => {
                    go.tco_enable_strict_const_semantics = parse_json(value);
                }
                b"require_types_tco_require_types_class_consts" => {
                    go.tco_require_types_class_consts = parse_json(value);
                }
                b"strict_wellformedness" => {
                    go.tco_strict_wellformedness = parse_json(value);
                }
                b"disable_hh_ignore_error" => {
                    go.po_disable_hh_ignore_error = parse_json(value);
                }
                b"allowed_fixme_codes_strict" => {
                    go.allowed_fixme_codes_strict = parse_iset(value);
                }
                b"allowed_decl_fixme_codes" => {
                    go.po_allowed_decl_fixme_codes = parse_iset(value);
                }
                b"allowed_files_for_module_declarations" => {
                    go.tco_allowed_files_for_module_declarations = parse_svec(value);
                }
                b"expression_tree_virtualize_functions" => {
                    go.tco_expression_tree_virtualize_functions = parse_json(value);
                }
                _ => c.unknown.push(parse_string(line)),
            }
        }
        c.hash = format!("{:x}", Sha1::digest(bytes));
        c
    }

    pub fn get_decl_parser_options(&self) -> DeclParserOptions {
        DeclParserOptions::from_parser_options(&self.opts)
    }
}

fn parse_json<'de, T: serde::de::Deserialize<'de>>(value: &'de [u8]) -> T {
    serde_json::from_slice(value).unwrap()
}

fn parse_string(value: &[u8]) -> String {
    String::from_utf8_lossy(value).into_owned()
}

fn parse_sset(value: &[u8]) -> BTreeSet<String> {
    parse_string(value)
        .split_terminator(',')
        .map(|s| s.trim().into())
        .collect()
}

fn parse_svec(value: &[u8]) -> Vec<String> {
    parse_string(value)
        .split_terminator(',')
        .map(|s| s.trim().into())
        .collect()
}

fn parse_iset(value: &[u8]) -> BTreeSet<isize> {
    parse_string(value)
        .split_terminator(',')
        .map(|s| s.trim().parse().unwrap())
        .collect()
}
