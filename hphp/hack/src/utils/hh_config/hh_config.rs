// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod local_config;

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Context;
use anyhow::Result;
use config_file::ConfigFile;
pub use local_config::LocalConfig;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::global_options::GlobalOptions;

pub const FILE_PATH_RELATIVE_TO_ROOT: &str = ".hhconfig";

/// For now, this struct only contains the parts of .hhconfig which
/// have been needed in Rust tools.
///
/// Fields correspond to ServerConfig.t
#[derive(Debug, Clone, Default)]
pub struct HhConfig {
    pub version: Option<String>,

    /// List of regex patterns of root-relative paths to ignore.
    pub ignored_paths: Vec<String>,

    /// SHA1 Hash of the .hhconfig file contents.
    pub hash: String,

    pub opts: GlobalOptions,
    pub local_config: LocalConfig,

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
    pub fn from_root(root: impl AsRef<Path>, overrides: &ConfigFile) -> Result<Self> {
        let hhconfig_path = root.as_ref().join(FILE_PATH_RELATIVE_TO_ROOT);
        let hh_conf_path = system_config_path();
        Self::from_files(hhconfig_path, hh_conf_path, overrides)
    }

    pub fn from_files(
        hhconfig_path: impl AsRef<Path>,
        hh_conf_path: impl AsRef<Path>,
        overrides: &ConfigFile,
    ) -> Result<Self> {
        let hhconfig_path = hhconfig_path.as_ref();
        let (hash, mut hhconfig) = ConfigFile::from_file_with_sha1(hhconfig_path)
            .with_context(|| hhconfig_path.display().to_string())?;
        hhconfig.apply_overrides(overrides);
        let hh_conf_path = hh_conf_path.as_ref();
        let mut hh_conf = ConfigFile::from_file(hh_conf_path)
            .with_context(|| hh_conf_path.display().to_string())?;
        hh_conf.apply_overrides(overrides);
        Ok(Self {
            hash,
            ..Self::from_configs(hhconfig, hh_conf)?
        })
    }

    pub fn from_slice(bytes: &[u8]) -> Result<Self> {
        let (hash, config) = ConfigFile::from_slice_with_sha1(bytes);
        Ok(Self {
            hash,
            ..Self::from_configs(config, Default::default())?
        })
    }

    /// Construct from .hhconfig and hh.conf files with CLI overrides already applied.
    pub fn from_configs(hhconfig: ConfigFile, hh_conf: ConfigFile) -> Result<Self> {
        let current_rolled_out_flag_idx = hhconfig
            .get_int("current_saved_state_rollout_flag_index")
            .unwrap_or(Ok(isize::MIN))?;

        let version = hhconfig.get_str("version");
        let mut c = Self {
            local_config: LocalConfig::from_config(version, current_rolled_out_flag_idx, hh_conf)?,
            ..Self::default()
        };

        // Some GlobalOptions fields are copied from LocalConfig
        let go = &mut c.opts;
        go.tco_saved_state = c.local_config.saved_state.clone();
        go.po_allow_unstable_features = c.local_config.allow_unstable_features;
        go.tco_rust_elab = c.local_config.rust_elab;

        for (key, mut value) in hhconfig {
            match key.as_str() {
                "auto_namespace_map" => {
                    let map: BTreeMap<String, String> = parse_json(&value)?;
                    go.po_auto_namespace_map = map.into_iter().collect();
                }
                "disable_xhp_element_mangling" => {
                    go.po_disable_xhp_element_mangling = parse_json(&value)?;
                }
                "disable_xhp_children_declarations" => {
                    go.po_disable_xhp_children_declarations = parse_json(&value)?;
                }
                "interpret_soft_types_as_like_types" => {
                    go.po_interpret_soft_types_as_like_types = parse_json(&value)?;
                }
                "everything_sdt" => {
                    go.tco_everything_sdt = parse_json(&value)?;
                }
                "deregister_php_stdlib" => {
                    go.po_deregister_php_stdlib = parse_json(&value)?;
                }
                "version" => {
                    c.version = Some(value);
                }
                "ignored_paths" => {
                    c.ignored_paths = parse_json(&value)?;
                }
                "enable_experimental_tc_features" => {
                    go.tco_experimental_features = parse_sset(&value);
                }
                "enable_xhp_class_modifier" => {
                    go.po_enable_xhp_class_modifier = parse_json(&value)?;
                }
                "disallow_invalid_arraykey" => {
                    go.tco_disallow_invalid_arraykey = parse_json(&value)?;
                }
                "check_xhp_attribute" => {
                    go.tco_check_xhp_attribute = parse_json(&value)?;
                }
                "disallow_silence" => {
                    go.po_disallow_silence = parse_json(&value)?;
                }
                "check_redundant_generics" => {
                    go.tco_check_redundant_generics = parse_json(&value)?;
                }
                "disallow_func_ptrs_in_constants" => {
                    go.po_disallow_func_ptrs_in_constants = parse_json(&value)?;
                }
                "enable_strict_string_concat_interp" => {
                    go.tco_enable_strict_string_concat_interp = parse_json(&value)?;
                }
                "allowed_expression_tree_visitors" => {
                    go.tco_allowed_expression_tree_visitors = parse_svec(&value);
                }
                "math_new_code" => {
                    go.tco_math_new_code = parse_json(&value)?;
                }
                "explicit_consistent_constructors" => {
                    go.tco_explicit_consistent_constructors = parse_json(&value)?;
                }
                "enable_strict_const_semantics" => {
                    go.tco_enable_strict_const_semantics = parse_json(&value)?;
                }
                "require_types_tco_require_types_class_consts" => {
                    go.tco_require_types_class_consts = parse_json(&value)?;
                }
                "strict_wellformedness" => {
                    go.tco_strict_wellformedness = parse_json(&value)?;
                }
                "disable_hh_ignore_error" => {
                    go.po_disable_hh_ignore_error = parse_json(&value)?;
                }
                "allowed_fixme_codes_strict" => {
                    go.allowed_fixme_codes_strict = parse_iset(&value)?;
                }
                "allowed_decl_fixme_codes" => {
                    go.po_allowed_decl_fixme_codes = parse_iset(&value)?;
                }
                "code_agnostic_fixme" => {
                    go.code_agnostic_fixme = parse_json(&value)?;
                }
                "allowed_files_for_module_declarations" => {
                    go.tco_allowed_files_for_module_declarations = parse_svec(&value);
                }
                "expression_tree_virtualize_functions" => {
                    go.tco_expression_tree_virtualize_functions = parse_json(&value)?;
                }
                "tco_global_access_check_enabled" => {
                    go.tco_global_access_check_enabled = parse_json(&value)?;
                }
                "log_levels" => {
                    go.log_levels = parse_json(&value)?;
                }
                "const_default_func_args" => {
                    go.po_const_default_func_args = parse_json(&value)?;
                }
                "const_default_lambda_args" => {
                    go.po_const_default_lambda_args = parse_json(&value)?;
                }
                "like_casts" => {
                    go.tco_like_casts = parse_json(&value)?;
                }
                "timeout" => {
                    go.tco_timeout = parse_json(&value)?;
                }
                "enable_sound_dynamic_type" => {
                    go.tco_enable_sound_dynamic = parse_json(&value)?;
                }
                "like_type_hints" => {
                    go.tco_like_type_hints = parse_json(&value)?;
                }
                "pessimise_builtins" => {
                    go.tco_pessimise_builtins = parse_json(&value)?;
                }
                "union_intersection_type_hints" => {
                    go.tco_union_intersection_type_hints = parse_json(&value)?;
                }
                "typecheck_sample_rate" => {
                    go.tco_typecheck_sample_rate = parse_json(&value)?;
                }
                "type_printer_fuel" => {
                    go.tco_type_printer_fuel = parse_json(&value)?;
                }
                "profile_top_level_definitions" => {
                    go.tco_profile_top_level_definitions = parse_json(&value)?;
                }
                "skip_check_under_dynamic" => {
                    go.tco_skip_check_under_dynamic = parse_json(&value)?;
                }
                "gc_minor_heap_size" => {
                    value.retain(|c| c != '_');
                    c.gc_minor_heap_size = parse_json(&value)?;
                }
                "gc_space_overhead" => {
                    c.gc_space_overhead = parse_json(&value)?;
                }
                "hackfmt.version" => {
                    c.hackfmt_version = parse_json(&value)?;
                }
                "sharedmem_dep_table_pow" => {
                    c.sharedmem_dep_table_pow = parse_json(&value)?;
                }
                "sharedmem_global_size" => {
                    value.retain(|c| c != '_');
                    c.sharedmem_global_size = parse_json(&value)?;
                }
                "sharedmem_hash_table_pow" => {
                    c.sharedmem_hash_table_pow = parse_json(&value)?;
                }
                "sharedmem_heap_size" => {
                    value.retain(|c| c != '_');
                    c.sharedmem_heap_size = parse_json(&value)?;
                }
                _ => c.unknown.push((key, value)),
            }
        }
        Ok(c)
    }

    pub fn get_decl_parser_options(&self) -> DeclParserOptions {
        DeclParserOptions::from_parser_options(&self.opts)
    }
}

fn parse_json<'de, T: serde::de::Deserialize<'de>>(value: &'de str) -> Result<T> {
    Ok(serde_json::from_slice(value.as_bytes())?)
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

fn parse_iset(value: &str) -> Result<BTreeSet<isize>> {
    value
        .split_terminator(',')
        .map(|s| Ok(s.trim().parse()?))
        .collect::<Result<_>>()
}

/// Return the local config file path, allowing HH_LOCALCONF_PATH to override it.
pub fn system_config_path() -> PathBuf {
    const HH_CONF: &str = "hh.conf";
    match std::env::var_os("HH_LOCALCONF_PATH") {
        Some(path) => Path::new(&path).join(HH_CONF),
        None => Path::new("/etc").join(HH_CONF), // TODO see options/buildOptions.ml for mac cfg
    }
}

#[cfg(test)]
mod test {
    use oxidized::saved_state_rollouts::SavedStateRollouts;

    use super::*;

    #[test]
    fn test_log_levels() {
        let hhconf = HhConfig::from_slice(br#"log_levels={ "pessimise": 1 }"#).unwrap();
        assert_eq!(
            hhconf.opts.log_levels.get("pessimise").copied(),
            Some(1isize)
        );
    }

    #[test]
    fn dummy_one() {
        let hhconfig =
            ConfigFile::from_args(["current_saved_state_rollout_flag_index=0".as_bytes()]);
        let hhconf = ConfigFile::from_args(["ss_force=candidate".as_bytes()]);
        let c = HhConfig::from_configs(hhconfig, hhconf).unwrap();
        assert_eq!(
            c.opts.tco_saved_state.rollouts,
            SavedStateRollouts {
                one: true,
                two: false,
                three: false,
            }
        );
    }

    #[test]
    fn dummy_two() {
        let hhconfig =
            ConfigFile::from_args(["current_saved_state_rollout_flag_index=0".as_bytes()]);
        let hhconf = ConfigFile::from_args(["ss_force=prod_with_flag_on:dummy_two".as_bytes()]);
        let c = HhConfig::from_configs(hhconfig, hhconf).unwrap();
        assert_eq!(
            c.opts.tco_saved_state.rollouts,
            SavedStateRollouts {
                one: false,
                two: true,
                three: false,
            }
        );
    }

    #[test]
    fn dummy_three() {
        let hhconfig =
            ConfigFile::from_args(["current_saved_state_rollout_flag_index=0".as_bytes()]);
        let hhconf = ConfigFile::from_args([
            "dummy_one=true".as_bytes(),
            "dummy_two=true".as_bytes(),
            "dummy_three=true".as_bytes(),
            "ss_force=prod_with_flag_on:dummy_three".as_bytes(),
        ]);
        let c = HhConfig::from_configs(hhconfig, hhconf).unwrap();
        assert_eq!(
            c.opts.tco_saved_state.rollouts,
            SavedStateRollouts {
                one: false,
                two: false,
                three: true,
            }
        );
    }

    #[test]
    fn dummy_three_err() {
        let hhconfig =
            ConfigFile::from_args(["current_saved_state_rollout_flag_index=0".as_bytes()]);
        let hhconf = ConfigFile::from_args([
            "ss_force=prod_with_myflag".as_bytes(), // bad ss_force syntax
        ]);
        let c = HhConfig::from_configs(hhconfig, hhconf);
        assert!(c.is_err())
    }

    #[test]
    fn dummy_one_prod() {
        let hhconfig =
            ConfigFile::from_args(["current_saved_state_rollout_flag_index=1".as_bytes()]);
        let hhconf = ConfigFile::from_args([
            "dummy_one=true".as_bytes(),
            "dummy_two=true".as_bytes(),
            "dummy_three=true".as_bytes(),
            "ss_force=prod".as_bytes(),
        ]);
        let c = HhConfig::from_configs(hhconfig, hhconf).unwrap();
        assert_eq!(
            c.opts.tco_saved_state.rollouts,
            SavedStateRollouts {
                one: true,
                two: false,
                three: false,
            }
        );
    }
}
