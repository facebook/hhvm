// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod local_config;

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::path::Path;
use std::path::PathBuf;
use std::str::FromStr;

use anyhow::Context;
use anyhow::Result;
use config_file::ConfigFile;
pub use local_config::LocalConfig;
use oxidized::custom_error_config::CustomErrorConfig;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::global_options::GlobalOptions;
use package::PackageInfo;
use sha1::Digest;
use sha1::Sha1;

pub const FILE_PATH_RELATIVE_TO_ROOT: &str = ".hhconfig";
pub const PACKAGE_FILE_PATH_RELATIVE_TO_ROOT: &str = "PACKAGES.toml";

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
    pub ide_fall_back_to_full_index: bool,
    pub hh_distc_should_disable_trace_store: bool,
    pub naming_table_compression_level: usize,
    pub naming_table_compression_threads: usize,
    pub eden_fetch_parallelism: usize,
}

impl HhConfig {
    pub fn from_root(root: impl AsRef<Path>, overrides: &ConfigFile) -> Result<Self> {
        let hhconfig_path = root.as_ref().join(FILE_PATH_RELATIVE_TO_ROOT);
        let hh_conf_path = system_config_path();
        Self::from_files(hhconfig_path, hh_conf_path, overrides)
    }

    pub fn create_packages_path(hhconfig_path: &Path) -> PathBuf {
        // Unwrap is safe because hhconfig_path is always at least one nonempty string
        let mut packages_path = hhconfig_path.parent().unwrap().to_path_buf();
        packages_path.push("PACKAGES.toml");
        packages_path
    }

    pub fn create_custom_errors_path(hhconfig_path: &Path) -> PathBuf {
        // Unwrap is safe because hhconfig_path is always at least one nonempty string
        let mut packages_path = hhconfig_path.parent().unwrap().to_path_buf();
        packages_path.push("CUSTOM_ERRORS.json");
        packages_path
    }

    pub fn from_files(
        hhconfig_path: impl AsRef<Path>,
        hh_conf_path: impl AsRef<Path>,
        overrides: &ConfigFile,
    ) -> Result<Self> {
        let hhconfig_path = hhconfig_path.as_ref();
        let package_config_pathbuf = Self::create_packages_path(hhconfig_path);
        let package_config_path = package_config_pathbuf.as_path();
        let custom_error_config_path = Self::create_custom_errors_path(hhconfig_path);
        let (contents, mut hhconfig) = ConfigFile::from_file_with_contents(hhconfig_path)
            .with_context(|| hhconfig_path.display().to_string())?;
        // Grab extra config and use it to process the hash
        let package_contents: String = if package_config_path.exists() {
            let ctxt = || package_config_path.display().to_string();
            let bytes = std::fs::read(package_config_path).with_context(ctxt)?;
            String::from_utf8(bytes).unwrap()
        } else {
            String::new()
        };
        let package_info: PackageInfo =
            PackageInfo::from_text(&package_contents).unwrap_or_default();
        let custom_error_contents: String = if custom_error_config_path.exists() {
            let ctxt = || custom_error_config_path.as_path().display().to_string();
            let bytes = std::fs::read(&custom_error_config_path).with_context(ctxt)?;
            String::from_utf8(bytes).unwrap()
        } else {
            "[]".to_string()
        };
        let hash = Self::hash(
            &hhconfig,
            &contents,
            &package_contents,
            &custom_error_contents,
        );
        hhconfig.apply_overrides(overrides);
        let hh_conf_path = hh_conf_path.as_ref();

        // We don't try to gracefully handle the case where the /etc/hh.conf
        // (or overridden path) does not exist on disk.
        //
        // There is no benefit: if someone makes a mistake with machine
        // configurations, we rather have everything blow up in our face fast
        // and tell us plainly what is going wrong than doing something "intelligent"
        // and falling back to some default configuration, which is bound to lead to
        // symptoms that are difficult to debug.
        let mut hh_conf = ConfigFile::from_file(hh_conf_path)
            .with_context(|| hh_conf_path.display().to_string())?;

        hh_conf.apply_overrides(overrides);
        let custom_error_config =
            CustomErrorConfig::from_str(&custom_error_contents).unwrap_or_default();
        Ok(Self {
            hash,
            ..Self::from_configs(hhconfig, hh_conf, custom_error_config, package_info)?
        })
    }

    pub fn into_config_files(
        root: impl AsRef<Path>,
    ) -> Result<(ConfigFile, ConfigFile, CustomErrorConfig, PackageInfo)> {
        let hhconfig_path = root.as_ref().join(FILE_PATH_RELATIVE_TO_ROOT);
        let hh_conf_path = system_config_path();
        let custom_error_config_pathbuf = Self::create_custom_errors_path(hhconfig_path.as_path());
        let custom_error_config_path = custom_error_config_pathbuf.as_path();
        let hh_config_file = ConfigFile::from_file(&hhconfig_path)
            .with_context(|| hhconfig_path.display().to_string())?;
        let hh_conf_file = ConfigFile::from_file(&hh_conf_path)
            .with_context(|| hh_conf_path.display().to_string())?;
        let custom_error_config = CustomErrorConfig::from_path(custom_error_config_path)?;
        let package_config_pathbuf = Self::create_packages_path(hhconfig_path.as_path());
        let package_config_path = package_config_pathbuf.as_path();
        let package_contents: String = if package_config_path.exists() {
            let ctxt = || package_config_path.display().to_string();
            let bytes = std::fs::read(package_config_path).with_context(ctxt)?;
            String::from_utf8(bytes).unwrap()
        } else {
            String::new()
        };
        let package_info: PackageInfo =
            PackageInfo::from_text(&package_contents).unwrap_or_default();

        Ok((
            hh_conf_file,
            hh_config_file,
            custom_error_config,
            package_info,
        ))
    }

    fn hash(
        parsed: &ConfigFile,
        config_contents: &str,
        package_config: &str,
        custom_error_config: &str,
    ) -> String {
        if let Some(hash) = parsed.get_str("override_hhconfig_hash") {
            return hash.to_owned();
        }
        let mut hasher = Sha1::new();
        hasher.update(config_contents.as_bytes());
        hasher.update(package_config.as_bytes());
        hasher.update(custom_error_config.as_bytes());
        format!("{:x}", hasher.finalize())
    }

    pub fn from_slice(bytes: &[u8]) -> Result<Self> {
        let (hash, config) = ConfigFile::from_slice_with_sha1(bytes);
        Ok(Self {
            hash,
            ..Self::from_configs(
                config,
                Default::default(),
                Default::default(),
                PackageInfo::default(),
            )?
        })
    }

    /// Construct from .hhconfig and hh.conf files with CLI overrides already applied.
    pub fn from_configs(
        hhconfig: ConfigFile,
        hh_conf: ConfigFile,
        custom_error_config: CustomErrorConfig,
        package_info: PackageInfo,
    ) -> Result<Self> {
        let current_rolled_out_flag_idx = hhconfig
            .get_int("current_saved_state_rollout_flag_index")
            .unwrap_or(Ok(0))?;
        let deactivate_saved_state_rollout = hhconfig
            .get_bool("deactivate_saved_state_rollout")
            .unwrap_or(Ok(false))?;

        let version = hhconfig.get_str("version");
        let mut c = Self {
            local_config: LocalConfig::from_config(
                version,
                current_rolled_out_flag_idx,
                deactivate_saved_state_rollout,
                &hh_conf,
            )?,
            ..Self::default()
        };

        // Some GlobalOptions fields are copied from LocalConfig
        let go = &mut c.opts;
        go.tco_saved_state = c.local_config.saved_state.clone();
        go.po_allow_unstable_features = c.local_config.allow_unstable_features;
        go.tco_rust_elab = c.local_config.rust_elab;
        go.tco_custom_error_config = custom_error_config;
        go.dump_tast_hashes = match hh_conf.get_str("dump_tast_hashes") {
            Some("true") => true,
            Some(_) | None => false,
        };
        go.dump_tasts = match hh_conf.get_str("dump_tasts") {
            None => vec![],
            Some(path) => {
                let path = PathBuf::from(path);
                let file = File::open(&path).with_context(|| path.to_string_lossy().to_string())?;
                BufReader::new(file)
                    .lines()
                    .collect::<std::io::Result<_>>()?
            }
        };
        // If there are errors, ignore them for the tcopt, the parser errors will be caught and
        // sent separately.
        go.tco_package_info = package_info.try_into().unwrap_or_default();

        for (key, mut value) in hhconfig {
            match key.as_str() {
                "current_saved_state_rollout_flag_index"
                | "deactivate_saved_state_rollout"
                | "override_hhconfig_hash"
                | "ss_force" => {
                    // These were already queried for LocalConfig above.
                    // Ignore them so they aren't added to c.unknown.
                }
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
                    let mut allowed_expression_tree_visitors = parse_svec(&value);
                    // Fix up type names so they will match with elaborated names.
                    // Keep this in sync with the Utils.add_ns loop in server/serverConfig.ml
                    for ty in &mut allowed_expression_tree_visitors {
                        if !ty.starts_with('\\') {
                            *ty = format!("\\{}", ty)
                        }
                    }
                    go.tco_allowed_expression_tree_visitors = allowed_expression_tree_visitors;
                }
                "locl_cache_capacity" => {
                    go.tco_locl_cache_capacity = parse_json(&value)?;
                }
                "locl_cache_node_threshold" => {
                    go.tco_locl_cache_node_threshold = parse_json(&value)?;
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
                "pessimise_builtins" => {
                    go.tco_pessimise_builtins = parse_json(&value)?;
                }
                "enable_no_auto_dynamic" => {
                    go.tco_enable_no_auto_dynamic = parse_json(&value)?;
                }
                "like_type_hints" => {
                    go.tco_like_type_hints = parse_json(&value)?;
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
                "ide_fall_back_to_full_index" => {
                    c.ide_fall_back_to_full_index = parse_json(&value)?;
                }
                "hh_distc_should_disable_trace_store" => {
                    c.hh_distc_should_disable_trace_store = parse_json(&value)?;
                }
                "naming_table_compression_threads" => {
                    c.naming_table_compression_threads = parse_json(&value)?;
                }
                "naming_table_compression_level" => {
                    c.naming_table_compression_level = parse_json(&value)?;
                }
                "log_exhaustivity_check" => {
                    go.tco_log_exhaustivity_check = parse_json(&value)?;
                }
                "dump_tast_hashes" => {
                    go.dump_tast_hashes = parse_json(&value)?;
                }
                "disallow_direct_superglobals_refs" => {
                    go.po_disallow_direct_superglobals_refs = parse_json(&value)?;
                }
                "eden_fetch_parallelism" => {
                    c.eden_fetch_parallelism = parse_json(&value)?;
                }
                "nameof_precedence" => {
                    go.po_nameof_precedence = parse_json(&value)?;
                }
                "strict_utf8" => {
                    go.po_strict_utf8 = parse_json(&value)?;
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
    use super::*;

    #[test]
    fn test_log_levels() {
        let hhconf = HhConfig::from_slice(br#"log_levels={ "pessimise": 1 }"#).unwrap();
        assert_eq!(
            hhconf.opts.log_levels.get("pessimise").copied(),
            Some(1isize)
        );
    }
}
