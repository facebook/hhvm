use std::path::Path;
use std::path::PathBuf;
use std::process::Command;

fn main() {
    // Assumes the hack workspace 'fbcode/hphp/hack/src/Cargo.toml'.
    let mut cargo_cmd = Command::new("cargo");
    cargo_cmd.args(&["locate-project", "--workspace", "--message-format=plain"]);
    let output = cargo_cmd.output().unwrap().stdout;
    let hphp = Path::new(std::str::from_utf8(&output).unwrap().trim())
        .ancestors()
        .nth(3)
        .unwrap();
    let fbcode = hphp.parent().unwrap();

    let files = vec![
        PathBuf::from("ffi_bridge.rs"),
        PathBuf::from("ffi_bridge.cpp"),
        PathBuf::from("ffi_bridge.h"),
        hphp.join("util/process-cpu.cpp"),
        hphp.join("util/process-cpu.h"),
        hphp.join("util/process-host.cpp"),
        hphp.join("util/process-host.h"),
    ];

    cxx_build::bridge("ffi_bridge.rs")
        .files(files.iter().filter(is_cpp))
        .include(fbcode)
        .define("NO_HHVM", "1")
        .warnings(false)
        .cpp(true)
        .flag("-std=c++17")
        .compile("ffi_bridge");

    files.iter().for_each(rerun_if_changed);
    rerun_if_changed("build.rs");
}

fn rerun_if_changed<P: AsRef<Path>>(f: P) {
    println!("cargo:rerun-if-changed={}", f.as_ref().to_str().unwrap());
}

fn is_cpp<P: AsRef<Path>>(path: &P) -> bool {
    path.as_ref().extension().map_or(false, |e| e == "cpp")
}
