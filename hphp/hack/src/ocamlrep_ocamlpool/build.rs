// Assume an opam environment (`eval "$(opam env --switch=default
// --set-switch)"`) then to find the prevailing standard library caml
// headers, `OCAMLLIB=$(ocamlopt.opt -config | grep standard_library:
// | awk '{ print $2 }')`.
fn ocamllib_dir() -> std::path::PathBuf {
    let mut sh = std::process::Command::new("sh");
    sh.args([
        "-c",
        "ocamlopt.opt -config | grep standard_library: | awk '{ print $2 }'",
    ]);
    std::path::Path::new(
        std::str::from_utf8(&sh.output().unwrap().stdout)
            .unwrap()
            .trim(),
    )
    .to_path_buf()
}

fn main() {
    // Tell Cargo that if the given file changes, to rerun this build script.
    println!("cargo:rerun-if-changed=ocamlpool.c");
    cc::Build::new()
        .include(ocamllib_dir().as_path().to_str().unwrap())
        .file("ocamlpool.c")
        .compile("ocamlpool");
}
