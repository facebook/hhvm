use std::path::Path;
use std::path::PathBuf;

fn main() {
    const ROOT_PATH: &str = "../../../../..";

    let root_path = Path::new(ROOT_PATH);
    let hphp_path = root_path.join("hphp");
    let neo_path = hphp_path.join("neo");

    let neo_files: Vec<PathBuf> = glob::glob(neo_path.join("*.c").to_str().unwrap())
        .unwrap()
        .collect::<Result<_, _>>()
        .unwrap();

    cc::Build::new()
        .files(&neo_files)
        .cpp(false)
        .include(&root_path)
        .warnings(false)
        .flag("-Wno-format")
        .compile("neo_hdf");

    neo_files.iter().for_each(rerun_if_changed);

    let hdf_files = vec![
        PathBuf::from("hdf.rs"),
        PathBuf::from("hdf-wrap.cpp"),
        PathBuf::from("hdf-wrap.h"),
        hphp_path.join("util/hdf.cpp"),
        hphp_path.join("util/hdf.h"),
        hphp_path.join("util/exception.cpp"),
        hphp_path.join("util/exception.h"),
        hphp_path.join("util/string-vsnprintf.cpp"),
        hphp_path.join("util/string-vsnprintf.h"),
    ];

    cxx_build::bridge("hdf.rs")
        .files(hdf_files.iter().filter(is_cpp))
        .include(&root_path)
        .define("NO_FOLLY", "1")
        .cpp(true)
        .flag("-std=c++17")
        .warnings(false)
        .compile("hdf");

    hdf_files.iter().for_each(rerun_if_changed);
    rerun_if_changed("build.rs");
}

fn rerun_if_changed<P: AsRef<Path>>(f: P) {
    println!("cargo:rerun-if-changed={}", f.as_ref().to_str().unwrap());
}

fn is_cpp<P: AsRef<Path>>(path: &P) -> bool {
    path.as_ref().extension().map_or(false, |e| e == "cpp")
}
