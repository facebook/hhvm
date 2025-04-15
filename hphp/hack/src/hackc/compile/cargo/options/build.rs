/// This file is used to generate output_gen.rs
use std::env;
use std::fs;
use std::path::Path;
use std::path::PathBuf;
use std::process::Command;
use std::process::ExitCode;

use nom::Err;
use nom::error::VerboseError;
use nom::error::convert_error;

fn rerun_if_changed<P: AsRef<Path>>(f: P) {
    println!("cargo:rerun-if-changed={}", f.as_ref().to_str().unwrap());
}

fn main() -> ExitCode {
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("Expected OUT_DIR in ENV"));

    // Assume the hack workspace 'fbcode/hphp/hack/src/Cargo.toml'.
    let mut cargo_cmd = Command::new("cargo");
    cargo_cmd.args(&["locate-project", "--workspace", "--message-format=plain"]);
    let output = cargo_cmd.output().unwrap().stdout;
    let hphp = Path::new(std::str::from_utf8(&output).unwrap().trim())
        .ancestors()
        .nth(3)
        .unwrap();

    let config_path = hphp.join("doc/configs.specification");

    let contents =
        fs::read_to_string(&config_path).expect("Should have been able to read the file");

    let res = generate_configs_lib::parse_option_doc::<VerboseError<&str>>(&contents);
    match res {
        Ok((_, sections)) => {
            generate_configs_lib::generate_hackc(sections, out_dir);
            rerun_if_changed("build.rs");
            rerun_if_changed(config_path);
            ExitCode::from(0)
        }
        Err(Err::Error(e)) | Err(Err::Failure(e)) => {
            println!(
                "error parsing header: {}",
                convert_error(contents.as_str(), e)
            );
            ExitCode::from(1)
        }
        Err(Err::Incomplete(_)) => ExitCode::from(2),
    }
}
