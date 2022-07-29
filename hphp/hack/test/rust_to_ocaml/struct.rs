pub struct MyStruct {
    pub foo: isize,
    pub bar: isize,
}

#[rust_to_ocaml(prefix = "a")]
pub struct StructA {
    pub foo: isize,
    pub bar: isize,
}

#[rust_to_ocaml(prefix = "b")]
pub struct StructB {
    pub foo: isize,
    pub bar: isize,
}
