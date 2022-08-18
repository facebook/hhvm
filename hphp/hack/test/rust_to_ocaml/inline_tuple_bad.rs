pub enum E {
    #[rust_to_ocaml(inline_tuple)]
    Foo(Box<A>),
}
