pub struct Foo(pub Bar, pub Bar);

#[rust_to_ocaml(and)]
pub struct Bar(pub Option<Foo>, pub Option<Foo>);
