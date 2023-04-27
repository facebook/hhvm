// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::path::PathBuf;

use files_to_ignore::FilesToIgnore;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::parser_options::ParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use rayon::prelude::*;
use relative_path::RelativePath;

#[derive(Debug, clap::Parser)]
pub struct Args {
    /// The root of the repository, where .hhconfig is, e.g., ~/www
    #[clap(long)]
    pub www: PathBuf,

    /// Filename to save naming table to
    #[clap(long)]
    pub output: PathBuf,

    /// Indicates that the output file should be overwritten, if present
    #[clap(long)]
    pub overwrite: bool,

    /// Provide a directory containing custom HHIs to use in place of the `hhi` crate
    #[clap(long)]
    pub custom_hhi_path: Option<PathBuf>,

    /// By default, this program exits with an error status code if the
    /// generated naming table contains duplicate symbols (stored in the
    /// NAMING_SYMBOLS_OVERFLOW table). If this option is provided, exit with a
    /// success code instead. hh_server is not designed to tolerate duplicates
    /// or read the overflow table, but the rearchitecture is.
    #[clap(long)]
    pub allow_collisions: bool,

    /// Write to the DB concurrently with parsing. Saves time but makes assigned
    /// file info IDs nondeterministic (by default, file info IDs are assigned
    /// so that RelativePaths are in sorted order).
    #[clap(long)]
    pub unsorted: bool,
}

pub fn build_naming_table(args: Args) -> anyhow::Result<ExitStatus> {
    let (log, _guard) = hh_slog::init_term_envlogger("");

    let hhconfig = hh_config::HhConfig::from_root(&args.www, &Default::default())?;

    if args.output.exists() {
        let output = args.output.display();
        if args.output.is_dir() {
            eprintln!("Cannot write to {output}; is a directory");
            return Ok(ExitStatus::InputError);
        }
        if args.overwrite {
            slog::warn!(log, "Deleting {output}, since --overwrite was passed",);
            std::fs::remove_file(&args.output)?;
        } else {
            eprintln!("{output} exists; if you wish to overwrite it, use the --overwrite flag",);
            return Ok(ExitStatus::InputError);
        }
    }

    let (hhi_path, _hhi_tmpdir) = if let Some(path) = args.custom_hhi_path {
        if path.exists() && path.is_dir() {
            slog::info!(log, "Using HHI files in {}", path.display());
            (path, None)
        } else {
            eprintln!("Custom HHI directory {} not found", path.display());
            return Ok(ExitStatus::InputError);
        }
    } else {
        let tmpdir = tempdir::TempDir::new("hh_naming_table_builder_hhi")?;
        let path = tmpdir.path().to_owned();
        slog::info!(log, "Extracting HHI files to {}", path.display());
        hhi::write_hhi_files(&path)?;
        (path, Some(tmpdir))
    };

    slog::info!(log, "Walking WWW...");
    let files_to_ignore = FilesToIgnore::new(&hhconfig.ignored_paths)?;
    let walk = |root, prefix| -> anyhow::Result<Vec<_>> {
        find_utils::find_hack_files(&files_to_ignore, root, prefix).collect()
    };
    let mut filenames = walk(&args.www, relative_path::Prefix::Root)?;
    filenames.extend(walk(&hhi_path, relative_path::Prefix::Hhi)?);

    let decl_opts = &DeclParserOptions::from_parser_options(&hhconfig.opts);
    let parse = |path| parse_file(&hhconfig.opts, decl_opts, &args.www, &hhi_path, path);
    let save_result = if args.unsorted {
        slog::info!(log, "Parsing files and writing to DB...");
        names::Names::build(&args.output, |tx| {
            filenames
                .into_par_iter()
                .try_for_each(|path| -> anyhow::Result<_> { Ok(tx.send(parse(path)?)?) })
        })?
    } else {
        slog::info!(log, "Parsing files...");
        let mut summaries: Vec<(RelativePath, names::FileSummary)> = filenames
            .into_par_iter()
            .map(parse)
            .collect::<anyhow::Result<_>>()?;
        summaries.par_sort_by(|a, b| a.0.cmp(&b.0));
        slog::info!(log, "Writing to DB...");
        names::Names::build_from_iterator(&args.output, summaries.into_iter())?
    };

    slog::info!(
        log,
        "Inserted symbols into the naming table: {:?}",
        &save_result
    );

    if !save_result.collisions.is_empty() && !args.allow_collisions {
        slog::error!(
            log,
            "Failed due to name collisions: {:?}",
            save_result.collisions,
        );
        return Ok(ExitStatus::SqlAssertionFailure);
    }

    slog::info!(log, "Finished saving naming table with 0 errors");
    Ok(ExitStatus::NoError)
}

#[derive(Copy, Clone, Debug)]
#[repr(u8)]
pub enum ExitStatus {
    NoError = 0,
    InputError = 10,
    SqlAssertionFailure = 212,
}

impl ExitStatus {
    pub fn as_code(self) -> i32 {
        self as u8 as i32
    }
}

fn parse_file(
    opts: &ParserOptions,
    decl_opts: &DeclParserOptions,
    root: &Path,
    hhi_path: &Path,
    path: RelativePath,
) -> anyhow::Result<(RelativePath, names::FileSummary)> {
    let text = std::fs::read(match path.prefix() {
        relative_path::Prefix::Root => root.join(path.path()),
        relative_path::Prefix::Hhi => hhi_path.join(path.path()),
        prefix => panic!("Unexpected RelativePath prefix: {prefix}"),
    })?;
    let deregister_php_stdlib_if_hhi = opts.po_deregister_php_stdlib;
    let prefix = path.prefix();
    let arena = bumpalo::Bump::new();
    let parsed_file =
        direct_decl_parser::parse_decls_for_typechecking(decl_opts, path.clone(), &text, &arena);
    let with_hashes =
        ParsedFileWithHashes::new(parsed_file, deregister_php_stdlib_if_hhi, prefix, &arena);
    let summary = names::FileSummary::new(&with_hashes);
    Ok((path, summary))
}
