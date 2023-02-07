// HACKC_ELAB_LOG=info buck2 run fbcode//hphp/hack/src/elab:hackc-elab -- files $HOME/fbsource/fbcode/hphp/hack/test/typecheck/tuple_hints.php $HOME/fbsource/fbcode/hphp/hack/test/typecheck/num_type_hint.php

//use log::info;
use std::path::PathBuf;

use anyhow::Result;
use byte_unit::Byte;
use clap::Parser;

/// Elaboration driver
#[derive(Parser, Debug, Default)]
struct Opts {
    #[clap(subcommand)]
    command: Option<Command>,
}

#[derive(Parser, Debug)]
enum Command {
    /// Perform elaboration on a list of hack source files
    Files(elab_files::Opts),
}

fn main() -> Result<()> {
    let mut builder = env_logger::Builder::from_env("HACKC_ELAB_LOG");
    builder.init();

    let mut opts = Opts::parse();

    rayon::ThreadPoolBuilder::new()
        .num_threads(0 /* use available */)
        .stack_size(Byte::from_str("32 MiB")?.get_bytes() as usize)
        .build_global()
        .unwrap();

    match opts.command.take() {
        Some(Command::Files(opts)) => elab_files::run(&opts),
        None => Ok(()),
    }
}

/// Elab driver
#[derive(Parser, Debug, Default)]
pub struct FileOpts {
    /// Input file(s)
    pub filenames: Vec<PathBuf>,
}

mod elab_files {
    use std::path::Path;

    use aast_parser::AastParser;
    use anyhow::anyhow;
    use anyhow::Result;
    use clap::Parser;
    use log::info;
    use ocamlrep::rc::RcOc;
    use parser_core_types::indexed_source_text::IndexedSourceText;
    use parser_core_types::source_text::SourceText;
    use rayon::prelude::*;
    use relative_path::Prefix;
    use relative_path::RelativePath;

    use super::FileOpts;

    #[derive(Parser, Debug)]
    pub struct Opts {
        #[clap(flatten)]
        pub files: FileOpts,
    }

    pub fn run(opts: &Opts) -> Result<()> {
        // Collect a Vec first so we process all files - not just up to the
        // first failure.
        let files = &opts.files.filenames;
        files
            .into_par_iter()
            .map(|path| elab_one_file(path, opts))
            .collect::<Vec<_>>()
            .into_iter()
            .collect()
    }

    #[derive(Clone, Default)]
    struct Ctx {}

    struct Err {}

    #[derive(Default)]
    struct NoPass {}
    impl transform::Pass for NoPass {
        type Ctx = Ctx;
        type Err = Err;
    }

    fn elab_one_file(path: &Path, _opts: &Opts) -> Result<()> {
        let content = std::fs::read(path)?;
        let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
        let filename = path.file_name().unwrap().to_str().unwrap();
        info!("proceeding to elaborate {filename:#?}");

        let source_text = SourceText::make(RcOc::new(filepath), &content);
        let indexed_source_text = IndexedSourceText::new(source_text);
        let env = aast_parser::rust_aast_parser_types::Env::default();
        match AastParser::from_text(&env, &indexed_source_text) {
            Err(error) => Err(anyhow!("parse failure {path:#?}: {error:#?}"))?,
            Ok(mut parse_result) => {
                info!("parsing {filename:#?}");
                let program = &mut parse_result.aast;
                let before = format!("{program:#?}");

                info!("calling 'tranform_ty_program' on {filename:#?}");
                let mut ctx = Ctx::default();
                let mut errs = Vec::default();
                let top_down = NoPass::default();
                let bottom_up = NoPass::default();
                transform::transform_ty_program(
                    program, &mut ctx, &mut errs, &top_down, &bottom_up,
                );
                if !errs.is_empty() {
                    println!(
                        "{} errors encountered elaborating {filename:#?}",
                        errs.len()
                    );
                }
                let after = format!("{program:#?}");

                info!("calculating diff for {filename:#?}");
                let diff = similar::TextDiff::from_lines(&before, &after)
                    .unified_diff()
                    .context_radius(10)
                    .header("before", "after")
                    .to_string();
                if diff.is_empty() {
                    info!("calling 'transform_ty_program' on {filename:#?} had no effect");
                }

                println!("diff ({path:#?}):\n{diff}");

                Ok(())
            }
        }
    }
}
