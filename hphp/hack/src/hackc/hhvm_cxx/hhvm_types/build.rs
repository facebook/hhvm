use std::path::Path;
use std::path::PathBuf;

fn main() {
    const ROOT_PATH: &str = "../../../../../..";

    let root_path = Path::new(ROOT_PATH);
    let hphp_path = root_path.join("hphp");

    let files = vec![
        PathBuf::from("hhvm_types_ffi.rs"),
        PathBuf::from("as-base-ffi.cpp"),
        PathBuf::from("as-base-ffi.h"),
        hphp_path.join("runtime/vm/as-base.cpp"),
        hphp_path.join("runtime/vm/as-base.h"),
    ];

    cxx_build::bridge("hhvm_types_ffi.rs")
        .files(files.iter().filter(is_cpp))
        .include(&root_path)
        .cpp(true)
        .flag("-std=c++17")
        .compile("hhvm_types_ffi");

    files.iter().for_each(rerun_if_changed);
    rerun_if_changed("build.rs");
}

fn rerun_if_changed<P: AsRef<Path>>(f: P) {
    println!("cargo:rerun-if-changed={}", f.as_ref().to_str().unwrap());
}

fn is_cpp<P: AsRef<Path>>(path: &P) -> bool {
    path.as_ref().extension().map_or(false, |e| e == "cpp")
}
