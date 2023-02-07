// HACKC_ELAB_LOG=info buck2 run fbcode//hphp/hack/src/elab:hackc-elab -- files $HOME/fbsource/fbcode/hphp/hack/test/typecheck/tuple_hints.php $HOME/fbsource/fbcode/hphp/hack/test/typecheck/num_type_hint.php

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
    use std::io::Write;
    use std::path::Path;

    use anyhow::Result;
    use clap::Parser;
    use rayon::prelude::*;

    use super::FileOpts;

    #[derive(Parser, Debug)]
    pub struct Opts {
        #[clap(flatten)]
        pub files: FileOpts,
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

    pub fn elab_one_file(path: &Path, _: &Opts) -> Result<()> {
        let top_down = NoPass::default();
        let bottom_up = NoPass::default();
        let diff = elab::elab_one_file(path, &top_down, &bottom_up)?;

        print!("diff:\n{diff}");
        std::io::stdout().flush().unwrap();

        Ok(())
    }
}
