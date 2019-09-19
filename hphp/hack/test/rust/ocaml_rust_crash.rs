use ocaml::caml;

caml!(rust_crash() {
    let _four = format!("{}", 4);
    ocaml::Value::unit()
});

caml!(rust_no_crash() {
    let _also_four = format!("{}", 4.to_string());
    ocaml::Value::unit()
});
