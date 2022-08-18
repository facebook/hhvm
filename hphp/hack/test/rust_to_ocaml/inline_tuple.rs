pub enum E {
    Foo((A, B)),
    Bar(Box<(A, B)>),
    #[rust_to_ocaml(inline_tuple)]
    Baz((A, B)),
    #[rust_to_ocaml(inline_tuple)]
    Qux(Box<(A, B)>),
}
