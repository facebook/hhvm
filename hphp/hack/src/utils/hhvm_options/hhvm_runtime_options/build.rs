use std::path::Path;
use std::path::PathBuf;

fn main() {
    const ROOT_PATH: &str = "../../../../../..";

    let root_path = Path::new(ROOT_PATH);
    let hphp_path = root_path.join("hphp");

    let files = vec![
        PathBuf::from("ffi_bridge.rs"),
        PathBuf::from("ffi_bridge.cpp"),
        PathBuf::from("ffi_bridge.h"),
        hphp_path.join("util/process-cpu.cpp"),
        hphp_path.join("util/process-cpu.h"),
        hphp_path.join("util/process-host.cpp"),
        hphp_path.join("util/process-host.h"),
    ];

    cxx_build::bridge("ffi_bridge.rs")
        .files(files.iter().filter(is_cpp))
        .include(&root_path)
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
