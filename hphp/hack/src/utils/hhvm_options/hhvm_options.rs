// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
mod hhvm_config;

use anyhow::anyhow;
use hdf::IniLine;
use hhvm_config::HhvmConfig;
use std::{ffi::OsStr, path::PathBuf};
use structopt::StructOpt;

// Define HHVM-compatible options, as best as we can with structopt.
#[derive(Debug, Default, StructOpt)]
pub struct HhvmOptions {
    /// Load specified HDF or INI config file(s)
    #[structopt(
        short("c"),
        long("config"),
        multiple(true),
        number_of_values(1),
        value_name("CONFIG")
    )]
    pub config_files: Vec<PathBuf>,

    /// Individual HDF configuration string
    #[structopt(
        short("v"),
        long("config-value"),
        multiple(true),
        number_of_values(1),
        value_name("NAME=VALUE")
    )]
    pub hdf_values: Vec<String>,

    /// Define an INI setting
    #[structopt(
        short("d"),
        long("define"),
        multiple(true),
        number_of_values(1),
        value_name("NAME=VALUE")
    )]
    pub ini_values: Vec<String>,
}

impl HhvmOptions {
    pub fn is_empty(&self) -> bool {
        self.config_files.is_empty() && self.hdf_values.is_empty() && self.ini_values.is_empty()
    }

    pub fn to_config(&self) -> anyhow::Result<HhvmConfig> {
        let mut hdf_config = hdf::Value::new();
        let mut ini_config = hdf::Value::new();

        for path in &self.config_files {
            let ext = path.extension();
            if ext == Some(OsStr::new("hdf")) {
                // ISSUE: The runtime will ignore these.
                hdf_config = hdf::Value::from_file(path)?;
                hdf_config =
                    hhvm_runtime_options::runtime_options::apply_tier_overrides(hdf_config)?;
            } else if ext == Some(OsStr::new("ini")) {
                // ISSUE: The runtime will process these but we should too.
                // ISSUE: Once we process these, perhaps we should just pass
                // down the resulting "-dKEY=VAL" options to the runtime, and
                // then we could delete some complex "core" runtime code.
                // Obviously this would also require "hnx" support too.
                ini_config = hdf::Value::from_ini_file(path)?;
            } else {
                return Err(anyhow!("{}: Unknown config file format", path.display(),));
            }
        }
        for opt in &self.hdf_values {
            match IniLine::parse(opt) {
                Err(_) | Ok(IniLine::Empty | IniLine::Section(_)) => {}
                Ok(IniLine::Key(_)) => {
                    // Silently ignored.
                }
                Ok(IniLine::KeyValue(k, v)) => {
                    hdf_config.set(k, v.into());
                }
            }
        }
        for opt in &self.ini_values {
            match IniLine::parse(opt) {
                Err(_) | Ok(IniLine::Empty | IniLine::Section(_)) => {}
                Ok(IniLine::Key(k)) => {
                    ini_config.set(k, "".into());
                }
                Ok(IniLine::KeyValue(k, v)) => {
                    ini_config.set(k, v.into());
                }
            }
        }

        Ok(HhvmConfig {
            hdf_config,
            ini_config,
        })
    }
}

// Compiler options that are compatible with hphp (hhvm --hphp),
// intended to be a CLI-compatible subset of HPHP::CompilerOptions
// in hphp/compiler/compiler.cpp.
#[derive(Debug, StructOpt)]
pub struct HphpOptions {
    #[structopt(flatten)]
    pub config: HhvmOptions,

    /// Compile target (ignored).
    #[structopt(
        long,
        short,
        default_value("hnbc"),
        possible_values(&["hnbc", "hnas", "hhbc"]),
    )]
    pub target: String,

    /// Output format (ignored).
    #[structopt(
        long,
        short,
        default_value("binary"),
        possible_values(&["binary", "hnas", "hnbc"]),
    )]
    pub format: String,

    /// Ignore code generation errors and continue compilation (ignored)
    #[structopt(long, parse(try_from_str = parse_boolish), default_value("true"))]
    pub force: bool,

    /// Log level (ignored - use HH_LOG). -1, 0: no logging; 1: errors, 2: warnings;
    /// 3: info, 4: verbose.
    #[structopt(long, short, default_value("-1"))]
    pub log: i32,

    /// Whether to save compilation stats and errors to output-dir/Stats.js (ignored)
    #[structopt(long, parse(try_from_str = parse_boolish), default_value("false"))]
    pub gen_stats: bool,

    /// Whether to keep the output directory (ignored)
    #[structopt(long, short, parse(try_from_str = parse_boolish), default_value("true"))]
    pub keep_tempdir: bool,

    /// Use the autoload map to parse additional files transitively referenced
    /// from input files.
    #[structopt(long, parse(try_from_str = parse_boolish), default_value("true"))]
    pub parse_on_demand: bool,

    /// Input directory. If specified, input pathnames are interpreted
    /// relative to this directory. Absolute input pathnames must have this
    /// directory as a prefix, which will be stripped.
    #[structopt(long, default_value(""))]
    pub input_dir: PathBuf,

    /// Output directory
    #[structopt(long, short)]
    pub output_dir: Option<PathBuf>,

    /// If specified, generate a static file cache with this filename (ignored)
    #[structopt(long)]
    pub file_cache: Option<PathBuf>,

    /// Directory containing input files.
    #[structopt(
        long("module"),
        multiple(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub modules: Vec<PathBuf>,

    /// Same as --module, except no exclusion checking is performed so these
    /// directories are forced to be included.
    #[structopt(
        long("fmodule"),
        multiple(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub fmodules: Vec<PathBuf>,

    /// Extra directories for static files without exclusion checking
    #[structopt(
        long("cmodule"),
        multiple(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub cmodules: Vec<PathBuf>,

    /// Extra static files force-included without exclusion checking (ignored)
    #[structopt(long("cfile"), multiple(true), number_of_values(1), value_name("PATH"))]
    pub cfiles: Vec<PathBuf>,

    /// Extra Hack source files force-included without exclusion checking (ignored)
    #[structopt(long("ffile"), multiple(true), number_of_values(1), value_name("PATH"))]
    pub ffiles: Vec<PathBuf>,

    /// Exclude these files or directories from the static content cache (ignored)
    #[structopt(
        long("exclude-static-pattern"),
        multiple(true),
        number_of_values(1),
        value_name("REGEX")
    )]
    pub exclude_static_patterns: Vec<String>,

    /// Directories to exclude from the input
    #[structopt(
        long("exclude-dir"),
        multiple(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub exclude_dirs: Vec<PathBuf>,

    /// Directories to exclude from the static content cache (ignored)
    #[structopt(
        long("exclude-static-dir"),
        multiple(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub exclude_static_dirs: Vec<PathBuf>,

    /// Regex pattern for files or directories to exclude from the input,
    /// even if --parse-on-demand finds it
    #[structopt(
        long("exclude-pattern"),
        multiple(true),
        number_of_values(1),
        value_name("REGEX")
    )]
    pub exclude_patterns: Vec<String>,

    /// Files to exclude from the input, even if parse-on-demand finds it
    #[structopt(
        long("exclude-file"),
        multiple(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub exclude_files: Vec<PathBuf>,

    /// Input file names
    pub inputs: Vec<PathBuf>,

    /// File containing list of relative file names, one per line.
    #[structopt(long, value_name("PATH"))]
    pub input_list: Option<PathBuf>,

    /// (ignored) By default, hhvm --hphp forks and does all the work in
    /// the child. This option can disable forking.
    #[structopt(long, parse(try_from_str = parse_boolish), default_value("false"))]
    pub nofork: bool,

    /// Filename of final program to emit; will be placed in output-dir.
    #[structopt(long)]
    pub program: Option<String>,
}

/// Parse strings that are convertible to bool. Currently:
/// true, false => bool
/// int => nonzero means true
fn parse_boolish(s: &str) -> std::result::Result<bool, std::num::ParseIntError> {
    s.parse::<bool>().or_else(|_| Ok(s.parse::<i64>()? != 0))
}
