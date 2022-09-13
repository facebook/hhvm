#![feature(exit_status_error)]

/// This file is used to build gen-rust.exe which we then use to build opcodes.rs
use std::env;
use std::path::PathBuf;
use std::process::Command;

use anyhow::Result;

fn main() -> Result<()> {
    let out_dir = PathBuf::from(env::var("OUT_DIR")?);
    let gen_rust_exe = out_dir.join("gen-rust.exe");

    // cc will normally only build a library - so we need to massage
    // it a little to build an exe.
    let mut build = cc::Build::new().cpp(true);
    build.include("../../..");
    let compiler = build.get_compiler();
    let mut cmd = compiler.to_command();
    cmd.arg("gen-rust.cpp");
    cmd.arg("-o");
    cmd.arg(&gen_rust_exe);
    cmd.spawn()?.wait()?.exit_ok()?;

    // Now that we have gen-rust.exe use it to generate opcodes.rs.
    let opcodes_out = Command::new(&gen_rust_exe).output()?;

    std::fs::write(out_dir.join("opcodes.rs"), opcodes_out.stdout)?;

    println!("cargo:rerun-if-changed=gen-rust.cpp");
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=../../runtime/vm/opcodes.h");
    Ok(())
}
