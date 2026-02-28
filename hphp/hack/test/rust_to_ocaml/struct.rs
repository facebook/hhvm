pub struct MyStruct {
    pub foo: isize,
    pub bar: isize,
}

#[rust_to_ocaml(prefix = "a_")]
pub struct StructA {
    pub foo: isize,
    pub bar: isize,
}

#[rust_to_ocaml(prefix = "b_")]
pub struct StructB {
    pub foo: isize,
    pub bar: isize,
}
