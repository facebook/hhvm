// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
mod hhvm_config;

use std::ffi::OsStr;
use std::path::PathBuf;

use anyhow::anyhow;
use anyhow::Result;
use clap::ArgAction;
use clap::Parser;
pub use hhvm_config::*;

// Define HHVM-compatible options, as best as we can with clap.
#[derive(Debug, Default, Parser)]
pub struct HhvmOptions {
    /// Load specified HDF or INI config file(s)
    #[clap(
        short('c'),
        long("config-file"),
        action(ArgAction::Append),
        value_name("CONFIG")
    )]
    pub config_files: Vec<PathBuf>,

    /// Individual HDF configuration string
    #[clap(
        short('v'),
        long("config-value"),
        action(ArgAction::Append),
        value_name("NAME=VALUE")
    )]
    pub hdf_values: Vec<String>,

    /// Define an INI setting
    #[clap(
        short('d'),
        long("define"),
        action(ArgAction::Append),
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
        value_parser = ["binary", "hhas", "text"]
    )]
    pub format: String,

    /// Log level (ignored - use HH_LOG). -1, 0: no logging; 1: errors, 2: warnings;
    /// 3: info, 4: verbose.
    #[clap(long, short, default_value("-1"))]
    pub log: i32,

    /// Input directory. If specified, input pathnames are interpreted
    /// relative to this directory. Absolute input pathnames must have this
    /// directory as a prefix, which will be stripped.
    #[clap(long)]
    pub input_dir: PathBuf,

    /// Output directory
    #[clap(long, short)]
    pub output_dir: Option<PathBuf>,

    /// If specified, generate a static file cache with this filename (ignored)
    #[clap(long)]
    pub file_cache: Option<PathBuf>,

    /// Directory containing input files.
    #[clap(long("dir"), action(ArgAction::Append), value_name("PATH"))]
    pub dirs: Vec<PathBuf>,

    /// Extra directories for static files without exclusion checking
    #[clap(long("cdir"), action(ArgAction::Append), value_name("PATH"))]
    pub cdirs: Vec<PathBuf>,

    /// Extra static files force-included without exclusion checking (ignored)
    #[clap(long("cfile"), action(ArgAction::Append), value_name("PATH"))]
    pub cfiles: Vec<PathBuf>,

    /// Exclude these files or directories from the static content cache (ignored)
    #[clap(
        long("exclude-static-pattern"),
        action(ArgAction::Append),
        value_name("REGEX")
    )]
    pub exclude_static_patterns: Vec<String>,

    /// Directories to exclude from the input
    #[clap(long("exclude-dir"), action(ArgAction::Append), value_name("PATH"))]
    pub exclude_dirs: Vec<PathBuf>,

    /// Directories to exclude from the static content cache (ignored)
    #[clap(
        long("exclude-static-dir"),
        action(ArgAction::Append),
        value_name("PATH")
    )]
    pub exclude_static_dirs: Vec<PathBuf>,

    /// Regex pattern for files or directories to exclude from the input,
    /// even if transitively referenced.
    #[clap(
        long("exclude-pattern"),
        action(ArgAction::Append),
        value_name("REGEX")
    )]
    pub exclude_patterns: Vec<String>,

    /// Files to exclude from the input, even if transitively referenced
    #[clap(long("exclude-file"), action(ArgAction::Append), value_name("PATH"))]
    pub exclude_files: Vec<PathBuf>,

    /// Input file names XXX (should this be --inputs?)
    pub inputs: Vec<PathBuf>,

    /// File containing list of relative file names, one per line.
    #[clap(long, value_name("PATH"))]
    pub input_list: Option<PathBuf>,

    /// Filename of final program to emit; will be placed in output-dir.
    #[clap(long)]
    pub program: Option<String>,
}
