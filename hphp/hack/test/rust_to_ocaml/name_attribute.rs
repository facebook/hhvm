#[rust_to_ocaml(name = "aa")] // ignored
type A = X;

#[rust_to_ocaml(name = "bb")] // ignored
struct B {
    #[rust_to_ocaml(name = "bb_x")]
    foo: x,
    #[rust_to_ocaml(name = "bb_y")]
    bar: y,
}

#[rust_to_ocaml(name = "cc")] // ignored
#[rust_to_ocaml(prefix = "C")]
enum C {
    #[rust_to_ocaml(name = "C_foo")]
    Foo,
    #[rust_to_ocaml(name = "Bar")]
    Bar {
        #[rust_to_ocaml(name = "bar_x")]
        foo: x,
        #[rust_to_ocaml(name = "bar_y")]
        bar: y,
    },
    Baz,
}

type a_alias = a;

type b_alias = b;

type c_alias = c;
