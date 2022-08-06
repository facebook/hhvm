// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
mod hhvm_config;

use std::ffi::OsStr;
use std::path::PathBuf;

use anyhow::anyhow;
use anyhow::Result;
use clap::Parser;
pub use hhvm_config::*;

// Define HHVM-compatible options, as best as we can with structopt.
#[derive(Debug, Default, Parser)]
pub struct HhvmOptions {
    /// Load specified HDF or INI config file(s)
    #[clap(
        short('c'),
        long("config"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("CONFIG")
    )]
    pub config_files: Vec<PathBuf>,

    /// Individual HDF configuration string
    #[clap(
        short('v'),
        long("config-value"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("NAME=VALUE")
    )]
    pub hdf_values: Vec<String>,

    /// Define an INI setting
    #[clap(
        short('d'),
        long("define"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("NAME=VALUE")
    )]
    pub ini_values: Vec<String>,
}

impl HhvmOptions {
    pub fn is_empty(&self) -> bool {
        self.config_files.is_empty() && self.hdf_values.is_empty() && self.ini_values.is_empty()
    }

    pub fn to_config(&self) -> Result<HhvmConfig> {
        let mut hdf_config = hdf::Value::default();
        let mut ini_config = hdf::Value::default();

        for path in &self.config_files {
            let ext = path.extension();
            if ext == Some(OsStr::new("hdf")) {
                hdf_config = hdf::Value::from_file(path)?;
                hdf_config =
                    hhvm_runtime_options::runtime_options::apply_tier_overrides(hdf_config)?;
            } else if ext == Some(OsStr::new("ini")) {
                ini_config = hdf::Value::from_ini_file(path)?;
            } else {
                return Err(anyhow!("{}: Unknown config file format", path.display(),));
            }
        }
        for opt in &self.hdf_values {
            hdf_config.set_hdf(opt)?;
        }
        for opt in &self.ini_values {
            ini_config.set_ini(opt)?;
        }

        Ok(HhvmConfig {
            hdf_config,
            ini_config,
        })
    }
}

// Compiler options that are compatible with hphp (hhvm --hphp),
// intended to be CLI-compatible subset with HPHP::CompilerOptions
// and prepareOptions() in hphp/compiler/compiler.cpp.
#[derive(Debug, Parser)]
pub struct HphpOptions {
    #[clap(flatten)]
    pub config: HhvmOptions,

    /// HHBC Output format (ignored).
    #[clap(
        long,
        short,
        default_value("binary"),
        possible_values(&["binary", "hhas", "text"]),
    )]
    pub format: String,

    /// Log level (ignored - use HH_LOG). -1, 0: no logging; 1: errors, 2: warnings;
    /// 3: info, 4: verbose.
    #[clap(long, short, default_value("-1"))]
    pub log: i32,

    /// Whether to keep the output directory (ignored)
    #[clap(long, short, parse(try_from_str = parse_boolish), default_value("true"))]
    pub keep_tempdir: bool,

    /// Use the autoload map to parse additional files transitively referenced
    /// from input files.
    #[clap(long, parse(try_from_str = parse_boolish), default_value("true"))]
    pub parse_on_demand: bool,

    /// Input directory. If specified, input pathnames are interpreted
    /// relative to this directory. Absolute input pathnames must have this
    /// directory as a prefix, which will be stripped.
    #[clap(long, default_value(""))]
    pub input_dir: PathBuf,

    /// Output directory
    #[clap(long, short)]
    pub output_dir: Option<PathBuf>,

    /// If specified, generate a static file cache with this filename (ignored)
    #[clap(long)]
    pub file_cache: Option<PathBuf>,

    /// Directory containing input files.
    #[clap(
        long("module"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub modules: Vec<PathBuf>,

    /// Same as --module, except no exclusion checking is performed so these
    /// directories are forced to be included.
    #[clap(
        long("fmodule"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub fmodules: Vec<PathBuf>,

    /// Extra directories for static files without exclusion checking
    #[clap(
        long("cmodule"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub cmodules: Vec<PathBuf>,

    /// Extra static files force-included without exclusion checking (ignored)
    #[clap(
        long("cfile"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub cfiles: Vec<PathBuf>,

    /// Extra Hack source files force-included without exclusion checking (ignored)
    #[clap(
        long("ffile"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub ffiles: Vec<PathBuf>,

    /// Exclude these files or directories from the static content cache (ignored)
    #[clap(
        long("exclude-static-pattern"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("REGEX")
    )]
    pub exclude_static_patterns: Vec<String>,

    /// Directories to exclude from the input
    #[clap(
        long("exclude-dir"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub exclude_dirs: Vec<PathBuf>,

    /// Directories to exclude from the static content cache (ignored)
    #[clap(
        long("exclude-static-dir"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub exclude_static_dirs: Vec<PathBuf>,

    /// Regex pattern for files or directories to exclude from the input,
    /// even if --parse-on-demand finds it
    #[clap(
        long("exclude-pattern"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("REGEX")
    )]
    pub exclude_patterns: Vec<String>,

    /// Files to exclude from the input, even if parse-on-demand finds it
    #[clap(
        long("exclude-file"),
        multiple_occurrences(true),
        number_of_values(1),
        value_name("PATH")
    )]
    pub exclude_files: Vec<PathBuf>,

    /// Input file names
    pub inputs: Vec<PathBuf>,

    /// File containing list of relative file names, one per line.
    #[clap(long, value_name("PATH"))]
    pub input_list: Option<PathBuf>,

    /// Filename of final program to emit; will be placed in output-dir.
    #[clap(long)]
    pub program: Option<String>,
}

/// Parse strings that are convertible to bool. Currently:
/// true, false => bool
/// int => nonzero means true
fn parse_boolish(s: &str) -> Result<bool, std::num::ParseIntError> {
    s.parse::<bool>().or_else(|_| Ok(s.parse::<i64>()? != 0))
}
