// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::Path;

use config_file::ConfigFile;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::global_options::GlobalOptions;

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

    /// Config settings that did not match any setting known to this parser.
    pub unknown: Vec<(String, String)>,

    pub gc_minor_heap_size: usize,
    pub gc_space_overhead: usize,
    pub hackfmt_version: usize,
    pub sharedmem_dep_table_pow: usize,
    pub sharedmem_global_size: usize,
    pub sharedmem_hash_table_pow: usize,
    pub sharedmem_heap_size: usize,
}

impl HhConfig {
    pub fn from_root(root: impl AsRef<Path>) -> std::io::Result<Self> {
        Self::from_file(root.as_ref().join(FILE_PATH_RELATIVE_TO_ROOT))
    }

    pub fn from_file(hhconfig_path: impl AsRef<Path>) -> std::io::Result<Self> {
        let (hash, config) = ConfigFile::from_file_with_sha1(hhconfig_path)?;
        Ok(Self {
            hash,
            ..Self::from_config(config)
        })
    }

    pub fn from_slice(bytes: &[u8]) -> Self {
        let (hash, config) = ConfigFile::from_slice_with_sha1(bytes);
        Self {
            hash,
            ..Self::from_config(config)
        }
    }

    pub fn from_config(config: ConfigFile) -> Self {
        let mut c = Self::default();
        for (key, mut value) in config {
            let go = &mut c.opts;
            match key.as_str() {
                "auto_namespace_map" => {
                    let map: BTreeMap<String, String> = parse_json(&value);
                    go.po_auto_namespace_map = map.into_iter().collect();
                }
                "disable_xhp_element_mangling" => {
                    go.po_disable_xhp_element_mangling = parse_json(&value);
                }
                "disable_xhp_children_declarations" => {
                    go.po_disable_xhp_children_declarations = parse_json(&value);
                }
                "interpret_soft_types_as_like_types" => {
                    go.po_interpret_soft_types_as_like_types = parse_json(&value);
                }
                "everything_sdt" => {
                    go.tco_everything_sdt = parse_json(&value);
                }
                "deregister_php_stdlib" => {
                    go.po_deregister_php_stdlib = parse_json(&value);
                }
                "version" => {
                    c.version = Some(value);
                }
                "ignored_paths" => {
                    c.ignored_paths = parse_json(&value);
                }
                "enable_experimental_tc_features" => {
                    go.tco_experimental_features = parse_sset(&value);
                }
                "enable_xhp_class_modifier" => {
                    go.po_enable_xhp_class_modifier = parse_json(&value);
                }
                "disallow_invalid_arraykey" => {
                    go.tco_disallow_invalid_arraykey = parse_json(&value);
                }
                "check_xhp_attribute" => {
                    go.tco_check_xhp_attribute = parse_json(&value);
                }
                "disallow_silence" => {
                    go.po_disallow_silence = parse_json(&value);
                }
                "check_redundant_generics" => {
                    go.tco_check_redundant_generics = parse_json(&value);
                }
                "disallow_func_ptrs_in_constants" => {
                    go.po_disallow_func_ptrs_in_constants = parse_json(&value);
                }
                "enable_strict_string_concat_interp" => {
                    go.tco_enable_strict_string_concat_interp = parse_json(&value);
                }
                "allowed_expression_tree_visitors" => {
                    go.tco_allowed_expression_tree_visitors = parse_svec(&value);
                }
                "math_new_code" => {
                    go.tco_math_new_code = parse_json(&value);
                }
                "explicit_consistent_constructors" => {
                    go.tco_explicit_consistent_constructors = parse_json(&value);
                }
                "enable_strict_const_semantics" => {
                    go.tco_enable_strict_const_semantics = parse_json(&value);
                }
                "require_types_tco_require_types_class_consts" => {
                    go.tco_require_types_class_consts = parse_json(&value);
                }
                "strict_wellformedness" => {
                    go.tco_strict_wellformedness = parse_json(&value);
                }
                "disable_hh_ignore_error" => {
                    go.po_disable_hh_ignore_error = parse_json(&value);
                }
                "allowed_fixme_codes_strict" => {
                    go.allowed_fixme_codes_strict = parse_iset(&value);
                }
                "allowed_decl_fixme_codes" => {
                    go.po_allowed_decl_fixme_codes = parse_iset(&value);
                }
                "code_agnostic_fixme" => {
                    go.code_agnostic_fixme = parse_json(&value);
                }
                "allowed_files_for_module_declarations" => {
                    go.tco_allowed_files_for_module_declarations = parse_svec(&value);
                }
                "expression_tree_virtualize_functions" => {
                    go.tco_expression_tree_virtualize_functions = parse_json(&value);
                }
                "tco_global_access_check_enabled" => {
                    go.tco_global_access_check_enabled = parse_json(&value);
                }
                "log_levels" => {
                    go.log_levels = parse_json(&value);
                }
                "allow_all_locations_for_type_constant_in_enum_class" => {
                    go.tco_allow_all_locations_for_type_constant_in_enum_class = parse_json(&value);
                }
                "allowed_locations_for_type_constant_in_enum_class" => {
                    go.tco_allowed_locations_for_type_constant_in_enum_class = parse_svec(&value);
                }
                "const_default_func_args" => {
                    go.po_const_default_func_args = parse_json(&value);
                }
                "const_default_lambda_args" => {
                    go.po_const_default_lambda_args = parse_json(&value);
                }
                "like_casts" => {
                    go.tco_like_casts = parse_json(&value);
                }
                "timeout" => {
                    go.tco_timeout = parse_json(&value);
                }
                "enable_sound_dynamic_type" => {
                    go.tco_enable_sound_dynamic = parse_json(&value);
                }
                "like_type_hints" => {
                    go.tco_like_type_hints = parse_json(&value);
                }
                "pessimise_builtins" => {
                    go.tco_pessimise_builtins = parse_json(&value);
                }
                "union_intersection_type_hints" => {
                    go.tco_union_intersection_type_hints = parse_json(&value);
                }
                "typecheck_sample_rate" => {
                    go.tco_typecheck_sample_rate = parse_json(&value);
                }
                "type_printer_fuel" => {
                    go.tco_type_printer_fuel = parse_json(&value);
                }
                "profile_top_level_definitions" => {
                    go.tco_profile_top_level_definitions = parse_json(&value);
                }
                "skip_check_under_dynamic" => {
                    go.tco_skip_check_under_dynamic = parse_json(&value);
                }
                "gc_minor_heap_size" => {
                    value.retain(|c| c != '_');
                    c.gc_minor_heap_size = parse_json(&value);
                }
                "gc_space_overhead" => {
                    c.gc_space_overhead = parse_json(&value);
                }
                "hackfmt.version" => {
                    c.hackfmt_version = parse_json(&value);
                }
                "sharedmem_dep_table_pow" => {
                    c.sharedmem_dep_table_pow = parse_json(&value);
                }
                "sharedmem_global_size" => {
                    value.retain(|c| c != '_');
                    c.sharedmem_global_size = parse_json(&value);
                }
                "sharedmem_hash_table_pow" => {
                    c.sharedmem_hash_table_pow = parse_json(&value);
                }
                "sharedmem_heap_size" => {
                    value.retain(|c| c != '_');
                    c.sharedmem_heap_size = parse_json(&value);
                }
                _ => c.unknown.push((key, value)),
            }
        }
        c
    }

    pub fn get_decl_parser_options(&self) -> DeclParserOptions {
        DeclParserOptions::from_parser_options(&self.opts)
    }
}

fn parse_json<'de, T: serde::de::Deserialize<'de>>(value: &'de str) -> T {
    serde_json::from_slice(value.as_bytes()).unwrap()
}

fn parse_sset(value: &str) -> BTreeSet<String> {
    value
        .split_terminator(',')
        .map(|s| s.trim().to_owned())
        .collect()
}

fn parse_svec(value: &str) -> Vec<String> {
    value
        .split_terminator(',')
        .map(|s| s.trim().to_owned())
        .collect()
}

fn parse_iset(value: &str) -> BTreeSet<isize> {
    value
        .split_terminator(',')
        .map(|s| s.trim().parse().unwrap())
        .collect()
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_log_levels() {
        let hhconf = HhConfig::from_slice(br#"log_levels={ "pessimise": 1 }"#);
        assert_eq!(
            hhconf.opts.log_levels.get("pessimise").copied(),
            Some(1isize)
        );
    }
}
